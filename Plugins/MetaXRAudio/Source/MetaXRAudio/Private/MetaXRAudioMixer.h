// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "AudioMixerDevice.h"
#include "Containers/Ticker.h"
#include "MetaXRAudioDllManager.h"
#include "MetaXR_Audio.h"
#include "Misc/Paths.h"

/************************************************************************/
/* MetaXRAudioSpatializationAudioMixer                                    */
/* This implementation of IAudioSpatialization uses the Meta XR Audio    */
/* library to render audio sources with HRTF spatialization.            */
/************************************************************************/
class UMetaXRAudioSourceSettings;
class METAXRAUDIO_API MetaXRAudioSpatializationAudioMixer : public IAudioSpatialization {
 public:
  MetaXRAudioSpatializationAudioMixer();
  ~MetaXRAudioSpatializationAudioMixer();

  void ClearContext();

  //~ Begin IAudioSpatialization
  virtual void Initialize(const FAudioPluginInitializationParams InitializationParams) override;
  virtual void Shutdown() override;
  virtual bool IsSpatializationEffectInitialized() const override;
  virtual void OnInitSource(const uint32 SourceId, const FName& AudioComponentUserId, USpatializationPluginSourceSettingsBase* InSettings)
      override;
  virtual void SetSpatializationParameters(uint32 VoiceId, const FSpatializationParams& InParams) override;
  virtual void ProcessAudio(const FAudioPluginSourceInputData& InputData, FAudioPluginSourceOutputData& OutputData) override;
  //~ End IAudioSpatialization

  bool Tick(float DeltaTime);

  static const int MIXER_CLASS_ID = 0x98765432;
  const int ClassID = MIXER_CLASS_ID;

 private:
  TArray<FSpatializationParams> Params;
  TArray<UMetaXRAudioSourceSettings*> SourceSettings;

  ovrAudioContext Context;
  FCriticalSection ContextLock;

  FAudioPluginInitializationParams InitParams;
  FTSTicker::FDelegateHandle TickDelegateHandle;
};
