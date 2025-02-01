// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetaXRAudioSourceSettings.h"
#include "MetaXRAudioDllManager.h"
#include "MetaXRAudioUtilities.h"

UMetaXRAudioSourceSettings::UMetaXRAudioSourceSettings()
    : EnableAcoustics(true),
      EnableSpatialization(true),
      GainBoostLevelDecibels(0.0f),
      ReverbSendLevelDecibels(0.0f),
      HRTFIntensity(1.0f),
      VolumetricRadius(0.0f),
      EarlyReflectionsSendLevelDecibels(0.0f),
      DirectivityPattern(EMetaXRAudioDirectivityPattern::None),
      DirectivityIntensity(1.0f),
      DirectSoundEnabled(true),
      ReverbReach(0.5f) {}

// Apply these source settings to a given context's source
void UMetaXRAudioSourceSettings::UpdateParameters(ovrAudioContext Context, uint32 ChangedSourceId) {
  uint32 Flags = 0;
  if (!EnableAcoustics)
    Flags |= ovrAudioSourceFlag_ReflectionsDisabled;

  ovrResult Result = OVRA_CALL(ovrAudio_SetAudioSourceFlags)(Context, ChangedSourceId, Flags);
  OVR_AUDIO_CHECK(Result, "Failed to set audio source flags");

  ovrAudioSourceAttenuationMode mode =
      EnableSpatialization ? ovrAudioSourceAttenuationMode_InverseSquare : ovrAudioSourceAttenuationMode_None;
  Result = OVRA_CALL(ovrAudio_SetAudioSourceAttenuationMode)(
      Context, ChangedSourceId, mode, MetaXRAudioUtilities::dbToLinear(GainBoostLevelDecibels));
  OVR_AUDIO_CHECK(Result, "Failed to set audio source attenuation mode");

  Result = OVRA_CALL(ovrAudio_SetAudioSourceRadius)(Context, ChangedSourceId, VolumetricRadius);
  OVR_AUDIO_CHECK(Result, "Failed to set audio source volumetric radius");

  Result = OVRA_CALL(ovrAudio_SetAudioReverbSendLevel)(Context, ChangedSourceId, MetaXRAudioUtilities::dbToLinear(ReverbSendLevelDecibels));
  OVR_AUDIO_CHECK(Result, "Failed to set reverb send level");

  Result = OVRA_CALL(ovrAudio_SetAudioHrtfIntensity)(Context, ChangedSourceId, HRTFIntensity);
  OVR_AUDIO_CHECK(Result, "Failed to set HRTF intensity");

  Result = OVRA_CALL(ovrAudio_SetAudioReflectionsSendLevel)(
      Context, ChangedSourceId, MetaXRAudioUtilities::dbToLinear(EarlyReflectionsSendLevelDecibels));
  OVR_AUDIO_CHECK(Result, "Failed to set early reflections send level");

  Result = OVRA_CALL(ovrAudio_SetSourceDirectivityEnabled)(
      Context, ChangedSourceId, DirectivityPattern == EMetaXRAudioDirectivityPattern::None ? 0 : 1);
  OVR_AUDIO_CHECK(Result, "Failed to set set source directivity enabled");

  Result = OVRA_CALL(ovrAudio_SetSourceDirectivityIntensity)(Context, ChangedSourceId, DirectivityIntensity);
  OVR_AUDIO_CHECK(Result, "Failed to set set source directivity intensity");

  Result = OVRA_CALL(ovrAudio_SetSourceDirectEnabled)(Context, ChangedSourceId, DirectSoundEnabled);
  OVR_AUDIO_CHECK(Result, "Failed to set set source direct sound enabled");

  Result = OVRA_CALL(ovrAudio_SetAudioSourceReverbReach)(Context, ChangedSourceId, ReverbReach);
  OVR_AUDIO_CHECK(Result, "Failed to set set source reverb reach");
}
