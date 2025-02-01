// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetaXRAmbisonicSpatializer.h"
#include "AudioMixerDevice.h"
#include "AudioPluginUtilities.h"
#include "DSP/FloatArrayMath.h"
#include "Engine/Engine.h"
#include "EngineGlobals.h"
#include "GameFramework/PlayerController.h"
#include "ISoundfieldFormat.h"
#include "MetaXRAudioContextManager.h"
#include "MetaXRAudioDllManager.h"
#include "MetaXRAudioMixer.h"
#include "MetaXRAudioUtilities.h"
#include "SoundFieldRendering.h"

// the Meta XR Audio renderer renders all inputs immediately to an interleaved stereo output.
class FMetaXRSoundfieldBuffer : public ISoundfieldAudioPacket {
 public:
  // Interleaved binaural audio buffer.
  Audio::FAlignedFloatBuffer AudioBuffer;
  int32 NumChannels;

  FMetaXRSoundfieldBuffer() : NumChannels(0) {}

  virtual void Serialize(FArchive& Ar) override {
    Ar << AudioBuffer;
    Ar << NumChannels;
  }

  virtual TUniquePtr<ISoundfieldAudioPacket> Duplicate() const override {
    return TUniquePtr<ISoundfieldAudioPacket>(new FMetaXRSoundfieldBuffer(*this));
  }

  virtual void Reset() override {
    AudioBuffer.Reset();
    NumChannels = 0;
  }
};

class FMetaXREncoder : public ISoundfieldEncoderStream {
 private:
  ovrAudioContext Context;
  TArray<int32> SourceIds;
  bool ShouldProcess;

  // temp buffers use to binauralize each source.
  Audio::FAlignedFloatBuffer ScratchMonoBuffer;
  Audio::FAlignedFloatBuffer ScratchOutputBuffer;

  static int32 SourceIdCounter;
  static FCriticalSection SourceIdCounterCritSection;

 public:
  FMetaXREncoder(ovrAudioContext InContext, int32 InNumChannels, int32 MaxNumSources) : Context(InContext) {
    // Encoding is only supported when the Meta XR Audio Spatializer is not in use.
    // If you create an encoder while it is in use, it will issue an error
    ShouldProcess = !MetaXRAudioUtilities::IsMetaXRAudioTheCurrentSpatializationPlugin();
    if (ShouldProcess) {
      check(Context != nullptr);
      {
        // Assign source Ids for each channel here.
        FScopeLock ScopeLock(&SourceIdCounterCritSection);
        for (int32 Index = 0; Index < InNumChannels; Index++) {
          SourceIds.Add(SourceIdCounter);
          SourceIdCounter = (SourceIdCounter + 1) % MaxNumSources;
        }
      }

      // Set default settings for each source channel here.
      for (int32& SourceId : SourceIds) {
        uint32 Flags = 0;
        Flags |= ovrAudioSourceFlag_ReflectionsDisabled;

        ovrResult Result = OVRA_CALL(ovrAudio_SetAudioSourceFlags)(Context, SourceId, Flags);
        check(Result == ovrSuccess);

        ovrAudioSourceAttenuationMode mode = ovrAudioSourceAttenuationMode_None;
        Result = OVRA_CALL(ovrAudio_SetAudioSourceAttenuationMode)(Context, SourceId, mode, 1.0f);
        check(Result == ovrSuccess);

        Result = OVRA_CALL(ovrAudio_SetAudioReverbSendLevel)(Context, SourceId, 0.0f);
        check(Result == ovrSuccess);
      }
    } else {
      UE_LOG(
          LogAudio,
          Error,
          TEXT(
              "Meta Ambisonic Encoding cannot be used while Meta XR Audio is set as the active spatialization plugin. It will output silence."));
    }
  }

  virtual void Encode(const FSoundfieldEncoderInputData& InputData, ISoundfieldAudioPacket& OutputData) override {
    OutputData.Reset();
    EncodeAndMixIn(InputData, OutputData);
  }

  // Binaurally spatialize each independent speaker position. If the spatialization plugin for the current platform was set to Meta XR Audio
  // when this encoder was created this will no-op.
  virtual void EncodeAndMixIn(const FSoundfieldEncoderInputData& InputData, ISoundfieldAudioPacket& OutputData) override {
    // Fill the output with stereo silence as baseline
    FMetaXRSoundfieldBuffer& OutputBuffer = DowncastSoundfieldRef<FMetaXRSoundfieldBuffer>(OutputData);
    const int32 NumFrames = InputData.AudioBuffer.Num() / InputData.NumChannels;
    OutputBuffer.AudioBuffer.Reset();
    OutputBuffer.NumChannels = 2;
    OutputBuffer.AudioBuffer.AddZeroed(NumFrames * OutputBuffer.NumChannels);

    if (ShouldProcess) {
      check(InputData.NumChannels == SourceIds.Num());

      // Find listener position and rotation: Translate into to OVR coordinates
      FVector DeviceRight = FVector::RightVector;
      FVector DeviceFront = FVector::ForwardVector;
      FVector DevicePosition = FVector::ZeroVector;
      if (GEngine) {
        TArray<APlayerController*> players;
        GEngine->GetAllLocalPlayerControllers(players);
        if (!players.IsEmpty())
          players[0]->GetAudioListenerPosition(DevicePosition, DeviceFront, DeviceRight);
      }
      FVector DeviceUp = DeviceFront.Cross(DeviceRight).GetUnsafeNormal();
      FVector OvrListenerPosition = MetaXRAudioUtilities::ToOVRVector(DevicePosition);
      FVector OvrListenerForward = MetaXRAudioUtilities::ToOVRVector(DeviceFront);
      FVector OvrListenerUp = MetaXRAudioUtilities::ToOVRVector(DeviceUp);

      ovrResult Result = OVRA_CALL(ovrAudio_SetListenerVectors)(
          Context,
          OvrListenerPosition.X,
          OvrListenerPosition.Y,
          OvrListenerPosition.Z,
          OvrListenerForward.X,
          OvrListenerForward.Y,
          OvrListenerForward.Z,
          OvrListenerUp.X,
          OvrListenerUp.Y,
          OvrListenerUp.Z);
      OVR_AUDIO_CHECK(Result, "Failed to set listener position and rotation");

      FQuat EmitterOrientation = InputData.PositionalData.Rotation;

      check(InputData.PositionalData.ChannelPositions);
      const TArray<Audio::FChannelPositionInfo>& ChannelPositions = *(InputData.PositionalData.ChannelPositions);

      for (int32 ChannelIndex = 0; ChannelIndex < SourceIds.Num(); ChannelIndex++) {
        int32& SourceId = SourceIds[ChannelIndex];
        const Audio::FChannelPositionInfo& ChannelPosition = ChannelPositions[ChannelIndex];

        // Translate the input position to OVR coordinates
        FVector SourcePosition = MetaXRAudioUtilities::ToOVRVector(ChannelPosition);
        FVector EmitterForward = MetaXRAudioUtilities::ToOVRVector(EmitterOrientation.GetForwardVector());
        FVector OvrUp = MetaXRAudioUtilities::ToOVRVector(EmitterOrientation.GetUpVector());
        // Set the source position and orientation
        Result = OVRA_CALL(ovrAudio_SetAudioSourceVectors)(
            Context,
            SourceId,
            SourcePosition.X,
            SourcePosition.Y,
            SourcePosition.Z,
            EmitterForward.X,
            EmitterForward.Y,
            EmitterForward.Z,
            OvrUp.X,
            OvrUp.Y,
            OvrUp.Z);
        OVR_AUDIO_CHECK(Result, "Failed to set emitter position and orientation");

        // Deinterleave the audio into the mono temp buffer.
        ScratchMonoBuffer.Reset();
        ScratchMonoBuffer.AddUninitialized(NumFrames);
        for (int32 FrameIndex = 0; FrameIndex < NumFrames; FrameIndex++) {
          ScratchMonoBuffer[FrameIndex] = InputData.AudioBuffer[FrameIndex * InputData.NumChannels + ChannelIndex];
        }

        ScratchOutputBuffer.Reset();
        ScratchOutputBuffer.AddUninitialized(NumFrames * 2);

        uint32 Status = 0;
        Result = OVRA_CALL(ovrAudio_SpatializeMonoSourceInterleaved)(
            Context, SourceId, &Status, ScratchOutputBuffer.GetData(), ScratchMonoBuffer.GetData());
        OVR_AUDIO_CHECK(Result, "Failed to spatialize mono source interleaved");

        // sum the interleaved output into the output bed.
        Audio::ArrayMixIn(ScratchOutputBuffer, OutputBuffer.AudioBuffer);
      } // end channel loop
    } // end if (ShouldProcess)
  } // end EncodeAndMixIn function implementation
}; // end Encoder class implementation

int32 FMetaXREncoder::SourceIdCounter = 0;
FCriticalSection FMetaXREncoder::SourceIdCounterCritSection;

// Because Meta XR spatializes every directly to an interleaved stereo buffer,
// we simply pass it forward here.
class FMetaXRAudioDecoder : public ISoundfieldDecoderStream {
 private:
 public:
  virtual void Decode(const FSoundfieldDecoderInputData& InputData, FSoundfieldDecoderOutputData& OutputData) override {
    OutputData.AudioBuffer.Reset();
    OutputData.AudioBuffer.AddZeroed(InputData.NumFrames * InputData.PositionalData.NumChannels);
    DecodeAndMixIn(InputData, OutputData);
  }

  virtual void DecodeAndMixIn(const FSoundfieldDecoderInputData& InputData, FSoundfieldDecoderOutputData& OutputData) override {
    const FMetaXRSoundfieldBuffer& InputBuffer = DowncastSoundfieldRef<const FMetaXRSoundfieldBuffer>(InputData.SoundfieldBuffer);

    if (InputBuffer.AudioBuffer.Num() == 0 || InputBuffer.NumChannels == 0) {
      return;
    } else if (InputData.PositionalData.NumChannels == InputBuffer.NumChannels) {
      // If the number of output channels is the same as the input channels, mix it in directly.
      Audio::ArrayMixIn(InputBuffer.AudioBuffer, OutputData.AudioBuffer);
    } else {
      // Otherwise, downmix and mix in.
      Audio::FAlignedFloatBuffer OutChannelMap;
      Audio::FMixerDevice::Get2DChannelMap(false, InputBuffer.NumChannels, InputData.PositionalData.NumChannels, false, OutChannelMap);
      Audio::DownmixAndSumIntoBuffer(
          InputBuffer.NumChannels,
          InputData.PositionalData.NumChannels,
          InputBuffer.AudioBuffer,
          OutputData.AudioBuffer,
          OutChannelMap.GetData());
    }
  }
};

// Class that takes ambisonics audio and renders it to an FMetaXRSoundfieldBuffer.
class FMetaXRAmbisonicsTranscoder : public ISoundfieldTranscodeStream {
 private:
  ovrAudioContext Context;
  ovrAudioAmbisonicStream AmbiStreamObject;

 public:
  FMetaXRAmbisonicsTranscoder(ovrAudioContext InContext, int32 InSampleRate, int32 InBufferLength) : Context(InContext) {
    ovrAudioAmbisonicFormat StreamFormat = ovrAudioAmbisonicFormat_AmbiX;
    ovrResult OurResult =
        OVRA_CALL(ovrAudio_CreateAmbisonicStream)(Context, InSampleRate, InBufferLength, StreamFormat, 1, &AmbiStreamObject);
    check(OurResult == 0);
    check(AmbiStreamObject != nullptr);
  }

  virtual void Transcode(
      const ISoundfieldAudioPacket& InputData,
      const ISoundfieldEncodingSettingsProxy& InputSettings,
      ISoundfieldAudioPacket& OutputData,
      const ISoundfieldEncodingSettingsProxy& OutputSettings) override {
    OutputData.Reset();
    TranscodeAndMixIn(InputData, InputSettings, OutputData, OutputSettings);
  }

  virtual void TranscodeAndMixIn(
      const ISoundfieldAudioPacket& InputData,
      const ISoundfieldEncodingSettingsProxy& InputSettings,
      ISoundfieldAudioPacket& PacketToSumTo,
      const ISoundfieldEncodingSettingsProxy& OutputSettings) override {
    const FAmbisonicsSoundfieldBuffer& InputBuffer = DowncastSoundfieldRef<const FAmbisonicsSoundfieldBuffer>(InputData);
    FMetaXRSoundfieldBuffer& OutputBuffer = DowncastSoundfieldRef<FMetaXRSoundfieldBuffer>(PacketToSumTo);

    if (!InputBuffer.NumChannels || InputBuffer.AudioBuffer.IsEmpty()) {
      return;
    }

    const int32 NumFrames = InputBuffer.AudioBuffer.Num() / InputBuffer.NumChannels;
    OutputBuffer.AudioBuffer.SetNumZeroed(NumFrames * 2);
    OutputBuffer.NumChannels = 2;

    // Currently, Meta XR only decodes first-order ambisonics to stereo.
    // in the future we can truncate InputBuffer to four channels and
    // render that.
    // For now we just output silence for higher order ambisonics from the Meta XR spatializaers
    if (InputBuffer.NumChannels == 4) {
      // Translate the input position to OVR coordinates
      FVector DeviceRight = FVector::RightVector;
      FVector DeviceFront = FVector::ForwardVector;
      FVector DevicePosition = FVector::ZeroVector;
      if (GEngine) {
        TArray<APlayerController*> players;
        GEngine->GetAllLocalPlayerControllers(players);
        if (!players.IsEmpty())
          players[0]->GetAudioListenerPosition(DevicePosition, DeviceFront, DeviceRight);
      }
      FVector DeviceUp = DeviceFront.Cross(DeviceRight).GetUnsafeNormal();
      FVector OvrListenerPosition = MetaXRAudioUtilities::ToOVRVector(DevicePosition);
      FVector OvrListenerForward = MetaXRAudioUtilities::ToOVRVector(DeviceFront);
      FVector OvrListenerUp = MetaXRAudioUtilities::ToOVRVector(DeviceUp);

      static auto SetListenerVectors = OVRA_CALL(ovrAudio_SetListenerVectors);
      ovrResult Result = SetListenerVectors(
          Context,
          OvrListenerPosition.X,
          OvrListenerPosition.Y,
          OvrListenerPosition.Z,
          OvrListenerForward.X,
          OvrListenerForward.Y,
          OvrListenerForward.Z,
          OvrListenerUp.X,
          OvrListenerUp.Y,
          OvrListenerUp.Z);

      // Find the rotation of the emitter source and use that to orient the ambisonic stream
      const FQuat& SourceRotation = InputBuffer.Rotation;
      FVector OvrSourceForward = MetaXRAudioUtilities::ToOVRVector(SourceRotation.GetForwardVector());
      FVector OvrSourceUp = MetaXRAudioUtilities::ToOVRVector(SourceRotation.GetUpVector());

      Result = OVRA_CALL(ovrAudio_SetAmbisonicOrientation)(
          AmbiStreamObject, OvrSourceForward.X, OvrSourceForward.Y, OvrSourceForward.Z, OvrSourceUp.X, OvrSourceUp.Y, OvrSourceUp.Z);

      ovrResult DecodeResult = OVRA_CALL(ovrAudio_ProcessAmbisonicStreamInterleaved)(
          Context, AmbiStreamObject, InputBuffer.AudioBuffer.GetData(), OutputBuffer.AudioBuffer.GetData(), NumFrames);
      check(DecodeResult == 0);
    }
  }
};

// Since FMetaXRSoundfieldBuffer is just an interleaved stereo buffer, we simply mix the buffer here.
class FMetaXRAudioSoundfieldMixer : public ISoundfieldMixerStream {
 public:
  virtual void MixTogether(const FSoundfieldMixerInputData& InputData, ISoundfieldAudioPacket& PacketToSumTo) override {
    const FMetaXRSoundfieldBuffer& InputBuffer = DowncastSoundfieldRef<const FMetaXRSoundfieldBuffer>(InputData.InputPacket);
    FMetaXRSoundfieldBuffer& OutputBuffer = DowncastSoundfieldRef<FMetaXRSoundfieldBuffer>(PacketToSumTo);

    if (OutputBuffer.AudioBuffer.Num() == 0) {
      OutputBuffer.AudioBuffer = InputBuffer.AudioBuffer;
      OutputBuffer.NumChannels = InputBuffer.NumChannels;
    } else {
      check(InputBuffer.AudioBuffer.Num() == OutputBuffer.AudioBuffer.Num());
      check(InputBuffer.NumChannels == OutputBuffer.NumChannels);

      Audio::ArrayMixIn(InputBuffer.AudioBuffer, OutputBuffer.AudioBuffer, InputData.SendLevel);
    }
  }
};

FMetaXRAmbisonicsFactory::FMetaXRAmbisonicsFactory() {
  ISoundfieldFactory::RegisterSoundfieldFormat(this);
}

FMetaXRAmbisonicsFactory::~FMetaXRAmbisonicsFactory() {
  ISoundfieldFactory::UnregisterSoundfieldFormat(this);
}

bool FMetaXRAmbisonicsFactory::IsTranscodeRequiredBetweenSettings(
    const ISoundfieldEncodingSettingsProxy& SourceSettings,
    const ISoundfieldEncodingSettingsProxy& DestinationSettings) {
  return false;
}

bool FMetaXRAmbisonicsFactory::CanTranscodeFromSoundfieldFormat(
    FName SourceFormat,
    const ISoundfieldEncodingSettingsProxy& SourceEncodingSettings) {
  FName AmbisonicsSoundfieldFormatName = GetUnrealAmbisonicsFormatName();
  if (SourceFormat == AmbisonicsSoundfieldFormatName) {
    return true;
  } else {
    return false;
  }
}

bool FMetaXRAmbisonicsFactory::CanTranscodeToSoundfieldFormat(
    FName DestinationFormat,
    const ISoundfieldEncodingSettingsProxy& DestinationEncodingSettings) {
  return false;
}

UClass* FMetaXRAmbisonicsFactory::GetCustomEncodingSettingsClass() const {
  return UMetaXRAudioSoundfieldSettings::StaticClass();
}

const USoundfieldEncodingSettingsBase* FMetaXRAmbisonicsFactory::GetDefaultEncodingSettings() {
  return GetDefault<UMetaXRAudioSoundfieldSettings>();
}

FName FMetaXRAmbisonicsFactory::GetSoundfieldFormatName() {
  static FName MetaXRBinauralFormatName = FName(TEXT("MetaXR Binaural"));
  return MetaXRBinauralFormatName;
}

TUniquePtr<ISoundfieldEncoderStream> FMetaXRAmbisonicsFactory::CreateEncoderStream(
    const FAudioPluginInitializationParams& InitInfo,
    const ISoundfieldEncodingSettingsProxy& InitialSettings) {
  ovrAudioContext Context = FMetaXRAudioContextManager::GetContextForAudioDevice(InitInfo.AudioDevicePtr);
  if (!Context) {
    Context = FMetaXRAudioContextManager::CreateContextForAudioDevice(InitInfo.AudioDevicePtr);
    check(Context);
  }

  return TUniquePtr<ISoundfieldEncoderStream>(
      new FMetaXREncoder(Context, InitInfo.NumOutputChannels, InitInfo.AudioDevicePtr->GetMaxChannels()));
}

TUniquePtr<ISoundfieldDecoderStream> FMetaXRAmbisonicsFactory::CreateDecoderStream(
    const FAudioPluginInitializationParams& InitInfo,
    const ISoundfieldEncodingSettingsProxy& InitialSettings) {
  return TUniquePtr<ISoundfieldDecoderStream>(new FMetaXRAudioDecoder());
}

TUniquePtr<ISoundfieldTranscodeStream> FMetaXRAmbisonicsFactory::CreateTranscoderStream(
    const FName SourceFormat,
    const ISoundfieldEncodingSettingsProxy& InitialSourceSettings,
    const FName DestinationFormat,
    const ISoundfieldEncodingSettingsProxy& InitialDestinationSettings,
    const FAudioPluginInitializationParams& InitInfo) {
  check(SourceFormat == GetUnrealAmbisonicsFormatName());
  check(DestinationFormat == GetSoundfieldFormatName());

  ovrAudioContext Context = FMetaXRAudioContextManager::GetContextForAudioDevice(InitInfo.AudioDevicePtr);
  if (!Context) {
    Context = FMetaXRAudioContextManager::CreateContextForAudioDevice(InitInfo.AudioDevicePtr);
    check(Context);
  }

  return TUniquePtr<ISoundfieldTranscodeStream>(new FMetaXRAmbisonicsTranscoder(Context, InitInfo.SampleRate, InitInfo.BufferLength));
}

TUniquePtr<ISoundfieldMixerStream> FMetaXRAmbisonicsFactory::CreateMixerStream(const ISoundfieldEncodingSettingsProxy& InitialSettings) {
  return TUniquePtr<ISoundfieldMixerStream>(new FMetaXRAudioSoundfieldMixer());
}

TUniquePtr<ISoundfieldAudioPacket> FMetaXRAmbisonicsFactory::CreateEmptyPacket() {
  return TUniquePtr<ISoundfieldAudioPacket>(new FMetaXRSoundfieldBuffer());
}
