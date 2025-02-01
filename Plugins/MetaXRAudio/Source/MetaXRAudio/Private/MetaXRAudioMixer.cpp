// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetaXRAudioMixer.h"
#include "IMetaXRAudioPlugin.h"
#include "MetaXRAudioContextManager.h"
#include "MetaXRAudioRoomAcousticProperties.h"
#include "MetaXRAudioSourceSettings.h"
#include "MetaXRAudioUtilities.h"
#include "Stats/Stats.h"

MetaXRAudioSpatializationAudioMixer::MetaXRAudioSpatializationAudioMixer() : Context(nullptr) {}

MetaXRAudioSpatializationAudioMixer::~MetaXRAudioSpatializationAudioMixer() {}

void MetaXRAudioSpatializationAudioMixer::ClearContext() {
  Context = nullptr;
}

void MetaXRAudioSpatializationAudioMixer::Initialize(const FAudioPluginInitializationParams InitializationParams) {
  FScopeLock ScopeLock(&ContextLock);
  InitParams = InitializationParams;

  Context = FMetaXRAudioContextManager::GetContextForAudioDevice(InitializationParams.AudioDevicePtr);
  if (!Context) {
    Context = FMetaXRAudioContextManager::CreateContextForAudioDevice(InitializationParams.AudioDevicePtr);
    check(Context);
  }

  Params.AddDefaulted(InitParams.NumSources);
  for (uint32 i = 0; i < InitParams.NumSources; i++) {
    SourceSettings.Add(nullptr);
  }

  // UE is in centimeters
  ovrResult Result = OVRA_CALL(ovrAudio_SetUnitScale)(Context, 0.01f);
  OVR_AUDIO_CHECK(Result, "Failed to set unit scale");

  TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &MetaXRAudioSpatializationAudioMixer::Tick));
}

void MetaXRAudioSpatializationAudioMixer::Shutdown() {
  if (TickDelegateHandle.IsValid()) {
    FTSTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
  }
}

bool MetaXRAudioSpatializationAudioMixer::IsSpatializationEffectInitialized() const {
  return Context != nullptr;
}

void MetaXRAudioSpatializationAudioMixer::OnInitSource(
    const uint32 SourceId,
    const FName& AudioComponentUserId,
    USpatializationPluginSourceSettingsBase* InSettings) {
  FScopeLock ScopeLock(&ContextLock);
  if (Context == nullptr) {
    UE_LOG(
        LogAudio,
        Error,
        TEXT("Meta XR Audio Error - Context uninitialized. Sound %s (id=%d) will not play!"),
        *AudioComponentUserId.ToString(),
        SourceId);
    return;
  }

  SourceSettings[SourceId] = nullptr;
  if (InSettings != nullptr) {
    UMetaXRAudioSourceSettings* Settings = CastChecked<UMetaXRAudioSourceSettings>(InSettings);
    SourceSettings[SourceId] = Settings;
  }
}

void MetaXRAudioSpatializationAudioMixer::SetSpatializationParameters(uint32 VoiceId, const FSpatializationParams& InParams) {
  Params[VoiceId] = InParams;
}

void MetaXRAudioSpatializationAudioMixer::ProcessAudio(
    const FAudioPluginSourceInputData& InputData,
    FAudioPluginSourceOutputData& OutputData) {
  FScopeLock ScopeLock(&ContextLock);
  if (InputData.SpatializationParams && Context) {
    Params[InputData.SourceId] = *InputData.SpatializationParams;

    // Update the source based on the source settings
    bool SpatializationEnabled = true;
    if (SourceSettings[InputData.SourceId] != nullptr) {
      SourceSettings[InputData.SourceId]->UpdateParameters(Context, InputData.SourceId);
      SpatializationEnabled = SourceSettings[InputData.SourceId]->EnableSpatialization;
    }

    // If spatialization is disabled, we can copy input to output and return
    if (!SpatializationEnabled) {
      // The output is interleaved stereo, collapse any number of channels input into mono and copy to output
      const float* InAudio = InputData.AudioBuffer->GetData();
      float* OutAudio = OutputData.AudioBuffer.GetData();
      const int32 NumInputChannels = InputData.NumChannels;
      const int32 NumOutputChannels = 2;
      int32 OutFrameIndex = 0;
      for (int32 InFrameIndex = 0; InFrameIndex < InputData.AudioBuffer->Num(); InFrameIndex += NumInputChannels) {
        for (int32 InChannelIndex = 0; InChannelIndex < NumInputChannels; InChannelIndex++) {
          OutAudio[OutFrameIndex] += InAudio[InFrameIndex + InChannelIndex];
          OutAudio[OutFrameIndex + 1] += InAudio[InFrameIndex + InChannelIndex];
        }
        OutFrameIndex += NumOutputChannels;
      }
      return;
    }

    // Translate the input position to OVR coordinates
    FVector OvrListenerPosition = MetaXRAudioUtilities::ToOVRVector(Params[InputData.SourceId].ListenerPosition);
    FVector OvrListenerForward = MetaXRAudioUtilities::ToOVRVector(Params[InputData.SourceId].ListenerOrientation.GetForwardVector());
    FVector OvrListenerUp = MetaXRAudioUtilities::ToOVRVector(Params[InputData.SourceId].ListenerOrientation.GetUpVector());

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

    // Translate the input position to OVR coordinates
    FVector OvrPosition = MetaXRAudioUtilities::ToOVRVector(Params[InputData.SourceId].EmitterWorldPosition);
    FVector OvrForward = MetaXRAudioUtilities::ToOVRVector(Params[InputData.SourceId].EmitterWorldRotation.GetForwardVector());
    FVector OvrUp = MetaXRAudioUtilities::ToOVRVector(Params[InputData.SourceId].EmitterWorldRotation.GetUpVector());

    // Set the source position and orientation
    Result = OVRA_CALL(ovrAudio_SetAudioSourceVectors)(
        Context,
        InputData.SourceId,
        OvrPosition.X,
        OvrPosition.Y,
        OvrPosition.Z,
        OvrForward.X,
        OvrForward.Y,
        OvrForward.Z,
        OvrUp.X,
        OvrUp.Y,
        OvrUp.Z);
    OVR_AUDIO_CHECK(Result, "Failed to set emitter position and orientation");

    // Perform the processing
    uint32 Status;
    Result = OVRA_CALL(ovrAudio_SpatializeMonoSourceInterleaved)(
        Context, InputData.SourceId, &Status, OutputData.AudioBuffer.GetData(), InputData.AudioBuffer->GetData());
    OVR_AUDIO_CHECK(Result, "Failed to spatialize mono source interleaved");
  }
}

bool MetaXRAudioSpatializationAudioMixer::Tick(float DeltaTime) {
  QUICK_SCOPE_CYCLE_COUNTER(STAT_MetaXRAudioSpatializationAudioMixer_Tick);
  if (ContextLock.TryLock()) {
    if (Context != nullptr) {
      UMetaXRAudioRoomAcousticProperties* RoomAcoustics = UMetaXRAudioRoomAcousticProperties::GetActiveRoomAcoustics();
      if (RoomAcoustics) {
        RoomAcoustics->UpdateRoomModel(Context);
      }
    }

    ContextLock.Unlock();
  }

  return true;
}
