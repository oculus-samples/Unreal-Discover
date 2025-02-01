// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IAudioExtensionPlugin.h"

#include "MetaXRAudioSourceSettings.generated.h"

UENUM()
enum class EMetaXRAudioDirectivityPattern : uint8 {
  None,
  HumanVoice,
};

UCLASS()
class METAXRAUDIO_API UMetaXRAudioSourceSettings : public USpatializationPluginSourceSettingsBase {
  GENERATED_BODY()

 public:
  UMetaXRAudioSourceSettings();

  // Apply these source settings to the specified source
  void UpdateParameters(ovrAudioContext Context, uint32 ChangedSourceId);

  // Enables room acoustics simulation (early reflections and reverberation) for this audio source only
  UPROPERTY(GlobalConfig, BlueprintReadWrite, EditAnywhere, Category = "SpatializationSettings")
  bool EnableAcoustics;

  // Enables HRTF Spatialization
  UPROPERTY(GlobalConfig, BlueprintReadWrite, EditAnywhere, Category = "SpatializationSettings")
  bool EnableSpatialization;

  // Additional gain beyond 0dB
  UPROPERTY(
      GlobalConfig,
      BlueprintReadWrite,
      EditAnywhere,
      Category = "SpatializationSettings",
      meta = (ClampMin = "0.0", ClampMax = "20.0", UIMin = "0.0", UIMax = "20.0"))
  float GainBoostLevelDecibels;

  // Additional gain applied to reverb send for this audio source only
  UPROPERTY(
      GlobalConfig,
      BlueprintReadWrite,
      EditAnywhere,
      Category = "SpatializationSettings",
      meta = (ClampMin = "-60.0", ClampMax = "20.0", UIMin = "-60.0", UIMax = "20.0"))
  float ReverbSendLevelDecibels;

  // How much of the HRTF EQ is applied to the sound. Interaural time delay (ITD) and interaural level differences (ILD) are kept the same.
  UPROPERTY(
      GlobalConfig,
      BlueprintReadWrite,
      EditAnywhere,
      Category = "ExperimentalSpatializationSettings",
      meta = (DisplayName = "HRTF Intensity", ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
  float HRTFIntensity;

  // Use to increase the spatial audio emitter radius. Useful for sounds that come from a large area rather than a precise point. If
  // increased too large, users may end up inside the radius if the sound source is too close.
  UPROPERTY(GlobalConfig, BlueprintReadWrite, EditAnywhere, Category = "ExperimentalSpatializationSettings", meta = (ClampMin = "0.0"))
  float VolumetricRadius;

  // Additional gain applied to early reflections for this audio source only
  UPROPERTY(
      GlobalConfig,
      BlueprintReadWrite,
      EditAnywhere,
      Category = "ExperimentalSpatializationSettings",
      meta = (ClampMin = "-60.0", ClampMax = "20.0", UIMin = "-60.0", UIMax = "20.0"))
  float EarlyReflectionsSendLevelDecibels;

  // Option for human voice directivity pattern that makes this sound more muffled when the source is facing away from listener
  UPROPERTY(GlobalConfig, BlueprintReadWrite, EditAnywhere, Category = "ExperimentalSpatializationSettings")
  EMetaXRAudioDirectivityPattern DirectivityPattern;

  // Intensity controller for Directvity , Value of 1 will apply full directivity
  UPROPERTY(
      GlobalConfig,
      BlueprintReadWrite,
      EditAnywhere,
      Category = "ExperimentalSpatializationSettings",
      meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
  float DirectivityIntensity;

  // If enabled, the direct sound for this source will be present in the output signal
  UPROPERTY(GlobalConfig, BlueprintReadWrite, EditAnywhere, Category = "ExperimentalSpatializationSettings")
  bool DirectSoundEnabled;

  // Adjust how much the direct-to-reverberant ratio increases with distance
  UPROPERTY(
      GlobalConfig,
      BlueprintReadWrite,
      EditAnywhere,
      Category = "ExperimentalSpatializationSettings",
      meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
  float ReverbReach;
};
