// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AudioDevice.h"
#include "IAudioExtensionPlugin.h"
#include "MetaXR_Audio.h"
#include "Sound/SoundEffectBase.h"
#include "Sound/SoundEffectSubmix.h"
#include "Templates/UniquePtr.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "MetaXRAudioReverb.generated.h"

// Forward Declarations
class USubmixEffectMetaXRReverbPluginPreset;

class METAXRAUDIO_API FSubmixEffectMetaXRReverbPlugin : public FSoundEffectSubmix {
  virtual void Init(const FSoundEffectSubmixInitData& InInitData) override;
  virtual uint32 GetDesiredInputChannelCountOverride() const override {
    static const int STEREO = 2;
    return STEREO; // PAS
  }
  virtual void OnProcessAudio(const FSoundEffectSubmixInputData& InData, FSoundEffectSubmixOutputData& OutData) override;
  virtual void OnPresetChanged() override {
    return; // PAS
  }

 public:
  FSubmixEffectMetaXRReverbPlugin();
  ~FSubmixEffectMetaXRReverbPlugin();

  void ClearContext();

 private:
  void OnNewDeviceCreated(Audio::FDeviceId InDeviceId);

  FDelegateHandle DeviceCreatedHandle;
  ovrAudioContext Context;
};

/************************************************************************/
/* MetaXRAudioReverb                                                    */
/* This implementation of IAudioReverb uses the Meta XR Audio            */
/* library to render spatial reverb.                                    */
/************************************************************************/
class MetaXRAudioReverb : public IAudioReverb {
 public:
  MetaXRAudioReverb() : Context(nullptr), ReverbPreset(nullptr) {
    // empty
  }

  void ClearContext();

  virtual void OnInitSource(
      const uint32 SourceId,
      const FName& AudioComponentUserId,
      const uint32 NumChannels,
      UReverbPluginSourceSettingsBase* InSettings) override {
    return; // PAS
  }

  virtual void OnReleaseSource(const uint32 SourceId) override {
    return; // PAS
  }

  virtual FSoundEffectSubmixPtr GetEffectSubmix() override;

  virtual USoundSubmix* GetSubmix() override;

  virtual void ProcessSourceAudio(const FAudioPluginSourceInputData& InputData, FAudioPluginSourceOutputData& OutputData) override {
    return; // PAS
  }

 private:
  ovrAudioContext* Context;
  TSoundEffectSubmixPtr SubmixEffect;
  USubmixEffectMetaXRReverbPluginPreset* ReverbPreset;
};

USTRUCT()
struct METAXRAUDIO_API FSubmixEffectMetaXRReverbPluginSettings {
  GENERATED_USTRUCT_BODY()

  FSubmixEffectMetaXRReverbPluginSettings() = default;
};

UCLASS()
class METAXRAUDIO_API USubmixEffectMetaXRReverbPluginPreset : public USoundEffectSubmixPreset {
  GENERATED_BODY()

 public:
  EFFECT_PRESET_METHODS(SubmixEffectMetaXRReverbPlugin)

  UFUNCTION()
  void SetSettings(const FSubmixEffectMetaXRReverbPluginSettings& InSettings) {}

  UPROPERTY()
  FSubmixEffectMetaXRReverbPluginSettings Settings;
};
