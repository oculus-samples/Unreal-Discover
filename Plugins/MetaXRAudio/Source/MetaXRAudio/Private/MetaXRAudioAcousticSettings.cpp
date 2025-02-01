// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.
#include "MetaXRAudioAcousticSettings.h"
#include "MetaXRAudioDllManager.h"

// Forward declare plugin helper function to set external audio systems metadata
extern "C" ovrResult ovrAudio_SetPluginReverbEnabled(bool reverbEnabled);
extern "C" ovrResult ovrAudio_SetPluginEarlyReflectionsEnabled(bool earlyReflectionsEnabled);
extern "C" ovrResult ovrAudio_SetPluginReverbSendLevel(float reverbSendLevel);

UMetaXRAudioAcousticSettings::UMetaXRAudioAcousticSettings()
    : EarlyReflectionsEnabled(true), ReverbEnabled(true), ReverbLevelDecibels(0.0f) {
  if (ActiveAcousticSettings) {
    UE_LOG(
        LogAudio,
        Warning,
        TEXT(
            "Meta XR Audio: Multiple Acoustic Settings components have been created. This is expected to be a singleton and creating multiple copies can result in unexpected behavior. The singleton will be set to this new component."));
  }

  ActiveAcousticSettings = this;

  // If we are using FMOD or Wwise, the acoustic settings will not update on a tick like Native Unreal.
  // Therefore, we must apply any editor settings immediately
  FMetaXRAudioLibraryManager::Get().UpdatePluginContext(0.0f);
}

void UMetaXRAudioAcousticSettings::BeginDestroy() {
  if (ActiveAcousticSettings == this) {
    ActiveAcousticSettings = nullptr;
  }

  Super::BeginDestroy();
}

UMetaXRAudioAcousticSettings* UMetaXRAudioAcousticSettings::GetActiveAcousticSettings() {
  return ActiveAcousticSettings;
}

// If we are using non-native audio system, this will allow us to update the acoustic settings metadata
void UMetaXRAudioAcousticSettings::UpdateAcousticSettings() {
  // FMOD / Wwise will internally update their metadata systems using the following function
  // Unreal native updates in the mixer process functions directly accessing the components settings
  // Because it doesn't need this function, unreal native will not find the function and skip this step
  auto setReverbEnabledFunction = OVRA_CALL(ovrAudio_SetPluginReverbEnabled);
  if (setReverbEnabledFunction != nullptr) {
    ovrResult result = setReverbEnabledFunction(ReverbEnabled);
    OVR_AUDIO_CHECK(result, "Failed to set Reverb Enable");
  }

  auto setEarlyReflectionsEnabledFunction = OVRA_CALL(ovrAudio_SetPluginEarlyReflectionsEnabled);
  if (setEarlyReflectionsEnabledFunction != nullptr) {
    ovrResult result = setEarlyReflectionsEnabledFunction(EarlyReflectionsEnabled);
    OVR_AUDIO_CHECK(result, "Failed to set Early Reflections Enable");
  }

  auto setReverbLevelFunction = OVRA_CALL(ovrAudio_SetPluginReverbSendLevel);
  if (setReverbLevelFunction != nullptr) {
    ovrResult result = setReverbLevelFunction(ReverbLevelDecibels);
    OVR_AUDIO_CHECK(result, "Failed to set Reverb Level");
  }
}

#if WITH_EDITOR
void UMetaXRAudioAcousticSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) {
  UpdateAcousticSettings();
  Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

// *************************************************************************************************
// Define blueprint setters here
// *************************************************************************************************
void UMetaXRAudioAcousticSettings::SetEarlyReflectionsEnabled(bool NewEarlyReflectionsEnabled) {
  if (NewEarlyReflectionsEnabled != EarlyReflectionsEnabled) {
    EarlyReflectionsEnabled = NewEarlyReflectionsEnabled;
    UpdateAcousticSettings();
  }
}

void UMetaXRAudioAcousticSettings::SetReverbEnabled(bool NewReverbEnabled) {
  if (NewReverbEnabled != ReverbEnabled) {
    ReverbEnabled = NewReverbEnabled;
    UpdateAcousticSettings();
  }
}

void UMetaXRAudioAcousticSettings::SetReverbLevelDecibels(float NewReverbLevelDecibels) {
  if (NewReverbLevelDecibels != ReverbLevelDecibels) {
    ReverbLevelDecibels = NewReverbLevelDecibels;
    UpdateAcousticSettings();
  }
}

// *************************************************************************************************
// Define blueprint getters here
// *************************************************************************************************
bool UMetaXRAudioAcousticSettings::GetEarlyReflectionsEnabled() const {
  return EarlyReflectionsEnabled;
}

bool UMetaXRAudioAcousticSettings::GetReverbEnabled() const {
  return ReverbEnabled;
}

float UMetaXRAudioAcousticSettings::GetReverbLevelDecibels() const {
  return ReverbLevelDecibels;
}
