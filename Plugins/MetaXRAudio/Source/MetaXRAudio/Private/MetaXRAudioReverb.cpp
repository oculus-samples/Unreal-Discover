// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetaXRAudioReverb.h"
#include "Engine/Engine.h"
#include "MetaXRAudioAcousticSettings.h"
#include "MetaXRAudioContextManager.h"
#include "MetaXRAudioMixer.h"
#include "MetaXRAudioSettings.h"
#include "MetaXRAudioUtilities.h"
#include "Sound/SoundSubmix.h"

#if !WITH_EDITOR
// In game we only have one context
static ovrAudioContext reverbContext;
#endif

namespace {
TSharedPtr<FSubmixEffectMetaXRReverbPlugin, ESPMode::ThreadSafe> CastEffectToPluginSharedPtr(FSoundEffectSubmixPtr InSubmixEffect) {
  return StaticCastSharedPtr<FSubmixEffectMetaXRReverbPlugin, FSoundEffectSubmix, ESPMode::ThreadSafe>(InSubmixEffect);
}
} // namespace

void FSubmixEffectMetaXRReverbPlugin::OnNewDeviceCreated(Audio::FDeviceId DeviceId) {
  FAudioDeviceManagerDelegates::OnAudioDeviceCreated.Remove(DeviceCreatedHandle);
  DeviceCreatedHandle.Reset();

  if (GEngine) {
    FAudioDevice* AudioDevice = GEngine->GetAudioDeviceManager()->GetAudioDeviceRaw(DeviceId);

    if (AudioDevice) {
      Context = FMetaXRAudioContextManager::GetContextForAudioDevice(AudioDevice);

      if (!Context) {
        Context = FMetaXRAudioContextManager::CreateContextForAudioDevice(AudioDevice);
      }
#if !WITH_EDITOR
      reverbContext = Context;
#endif
    }
  }
}

void FSubmixEffectMetaXRReverbPlugin::ClearContext() {
  Context = nullptr;
}

void FSubmixEffectMetaXRReverbPlugin::Init(const FSoundEffectSubmixInitData& InInitData) {
  DeviceCreatedHandle =
      FAudioDeviceManagerDelegates::OnAudioDeviceCreated.AddRaw(this, &FSubmixEffectMetaXRReverbPlugin::OnNewDeviceCreated);
}

FSubmixEffectMetaXRReverbPlugin::FSubmixEffectMetaXRReverbPlugin() : Context(nullptr) {}

FSubmixEffectMetaXRReverbPlugin::~FSubmixEffectMetaXRReverbPlugin() {
  if (DeviceCreatedHandle.IsValid()) {
    FAudioDeviceManagerDelegates::OnAudioDeviceCreated.Remove(DeviceCreatedHandle);
    DeviceCreatedHandle.Reset();
  }
}

void FSubmixEffectMetaXRReverbPlugin::OnProcessAudio(
    const FSoundEffectSubmixInputData& InputData,
    FSoundEffectSubmixOutputData& OutputData) {
  if (InputData.AudioBuffer)
    memcpy(OutputData.AudioBuffer->GetData(), InputData.AudioBuffer->GetData(), InputData.AudioBuffer->Num() * sizeof(float));
  else
    memset(OutputData.AudioBuffer->GetData(), 0, OutputData.AudioBuffer->Num() * sizeof(float));

  ovrAudioContext currentContext;
#if !WITH_EDITOR
  currentContext = reverbContext;
#else
  currentContext = Context;
#endif

  if (currentContext) {
    uint32_t Status = 0;
    ovrResult Result;
    const UMetaXRAudioAcousticSettings* Settings = UMetaXRAudioAcousticSettings::GetActiveAcousticSettings();

    if (Settings != nullptr) {
      // First apply any new settings here before processing
      Result = OVRA_CALL(ovrAudio_Enable)(currentContext, ovrAudioEnable_SimpleRoomModeling, Settings->EarlyReflectionsEnabled);
      OVR_AUDIO_CHECK(Result, "Failed to enable reflections");

      Result = OVRA_CALL(ovrAudio_Enable)(currentContext, ovrAudioEnable_LateReverberation, Settings->ReverbEnabled);
      OVR_AUDIO_CHECK(Result, "Failed to enable reverb");

      Result = OVRA_CALL(ovrAudio_SetSharedReverbWetLevel)(currentContext, MetaXRAudioUtilities::dbToLinear(Settings->ReverbLevelDecibels));
      OVR_AUDIO_CHECK(Result, "Failed to set room parameters");
    }

    // Now render output
    Result = OVRA_CALL(ovrAudio_MixInSharedReverbInterleaved)(currentContext, &Status, OutputData.AudioBuffer->GetData());
    OVR_AUDIO_CHECK(Result, "Failed to process reverb");
  }
}

void MetaXRAudioReverb::ClearContext() {
  Context = nullptr;
  if (SubmixEffect.IsValid()) {
    CastEffectToPluginSharedPtr(SubmixEffect)->ClearContext();
  }
}

FSoundEffectSubmixPtr MetaXRAudioReverb::GetEffectSubmix() {
  if (!SubmixEffect.IsValid()) {
    if (!ReverbPreset) {
      ReverbPreset = NewObject<USubmixEffectMetaXRReverbPluginPreset>();
      ReverbPreset->AddToRoot();
    }

    SubmixEffect =
        USoundEffectPreset::CreateInstance<FSoundEffectSubmixInitData, FSoundEffectSubmix>(FSoundEffectSubmixInitData(), *ReverbPreset);
    SubmixEffect->SetEnabled(true);
  }

  return SubmixEffect;
}

USoundSubmix* MetaXRAudioReverb::GetSubmix() {
  const UMetaXRAudioSettings* Settings = GetDefault<UMetaXRAudioSettings>();
  check(Settings);
  USoundSubmix* ReverbSubmix = Cast<USoundSubmix>(Settings->OutputSubmix.TryLoad());
  if (!ReverbSubmix) {
    static const FString DefaultSubmixName = TEXT("MetaXR Reverb Submix");
    UE_LOG(
        LogAudio,
        Warning,
        TEXT("Failed to load MetaXR Reverb Submix from object path '%s' in MetaXRSettings. Creating '%s' as stub."),
        *Settings->OutputSubmix.GetAssetPathString(),
        *DefaultSubmixName);

    ReverbSubmix = NewObject<USoundSubmix>(USoundSubmix::StaticClass(), *DefaultSubmixName);
    ReverbSubmix->bMuteWhenBackgrounded = true;
  }
  ReverbSubmix->bAutoDisable = false;

  return ReverbSubmix;
}
