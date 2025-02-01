// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetaXRAudioContextManager.h"
#include "AudioMixerDevice.h"
#include "Features/IModularFeatures.h"
#include "IMetaXRAudioPlugin.h"
#include "MetaXRAmbisonicSpatializer.h"
#include "MetaXRAudioReverb.h"
#include "MetaXRAudioSettings.h"

ovrAudioContext FMetaXRAudioContextManager::SerializationContext = nullptr;
UActorComponent* FMetaXRAudioContextManager::SerializationParent = nullptr;

FCriticalSection FMetaXRAudioContextManager::SerializationContextLock;

TMap<const FAudioDevice*, ovrAudioContext> FMetaXRAudioContextManager::ContextMap;
FCriticalSection FMetaXRAudioContextManager::ContextCritSection;

FMetaXRAudioContextManager::FMetaXRAudioContextManager() {
  // empty
}

FMetaXRAudioContextManager::~FMetaXRAudioContextManager() {
  SerializationContext = nullptr;
}

void FMetaXRAudioContextManager::OnListenerInitialize(FAudioDevice* AudioDevice, UWorld* ListenerWorld) {}

void FMetaXRAudioContextManager::OnListenerShutdown(FAudioDevice* AudioDevice) {
  FMetaXRAudioPlugin* Plugin = &FModuleManager::GetModuleChecked<FMetaXRAudioPlugin>("MetaXRAudio");
  const FString MetaXRSpatializerPluginName = Plugin->GetSpatializationPluginFactory()->GetDisplayName();
  const FString CurrentSpatializerPluginName = AudioPluginUtilities::GetDesiredPluginName(EAudioPlugin::SPATIALIZATION);
  if (CurrentSpatializerPluginName.Equals(MetaXRSpatializerPluginName)) // we have a match!
  {
    MetaXRAudioSpatializationAudioMixer* Spatializer =
        static_cast<MetaXRAudioSpatializationAudioMixer*>(AudioDevice->GetSpatializationPluginInterface().Get());

    Spatializer->ClearContext();
  }

  const FString MetaXRReverbPluginName = Plugin->GetReverbPluginFactory()->GetDisplayName();
  const FString CurrentReverbPluginName = AudioPluginUtilities::GetDesiredPluginName(EAudioPlugin::REVERB);
  if (CurrentReverbPluginName.Equals(MetaXRReverbPluginName)) {
    MetaXRAudioReverb* Reverb = static_cast<MetaXRAudioReverb*>(AudioDevice->ReverbPluginInterface.Get());
    Reverb->ClearContext();
  }

  // FIXME:
  // There's a possibility this will leak if a Meta XR Binaural submix is created,
  // but Meta XR audio is not specified as the spatialization or reverb plugin.
  // This is a niche use case, but could be solved by having the Meta XR soundfield explicitly destroy
  // a context.
  DestroyContextForAudioDevice(AudioDevice);
  SerializationContext = nullptr;
}

ovrAudioContext FMetaXRAudioContextManager::GetOrCreateSerializationContext(UActorComponent* Parent) {
  FScopeLock ScopeLock(&SerializationContextLock);

  ovrAudioContext PluginContext = FMetaXRAudioLibraryManager::Get().GetPluginContext();
  if (PluginContext != nullptr) {
    return PluginContext;
  }

  if (SerializationContext == nullptr) {
    ovrResult Result = OVRA_CALL(ovrAudio_CreateContext)(&SerializationContext, nullptr);
    if (Result != ovrSuccess) {
      const TCHAR* ErrString = GetMetaXRErrorString(Result);
      UE_LOG(
          LogAudio,
          Error,
          TEXT("Meta XR Audio SDK Error - %s: %s"),
          TEXT("Failed to create Meta XR Audio context for serialization"),
          ErrString);
      return nullptr;
    }

    SerializationParent = Parent;
  }

  return SerializationContext;
}

ovrAudioContext FMetaXRAudioContextManager::GetContextForAudioDevice(const FAudioDevice* InAudioDevice) {
  FScopeLock ScopeLock(&ContextCritSection);
  ovrAudioContext* Context = ContextMap.Find(InAudioDevice);
  if (Context) {
    return *Context;
  } else {
    return nullptr;
  }
}

ovrAudioContext FMetaXRAudioContextManager::CreateContextForAudioDevice(FAudioDevice* InAudioDevice) {
  const UMetaXRAudioSettings* Settings = GetDefault<UMetaXRAudioSettings>();

  ovrAudioContextConfiguration ContextConfig = {0};
  ContextConfig.acc_BufferLength = InAudioDevice->GetBufferLength();
  ContextConfig.acc_MaxNumSources = Settings->VoiceLimit;
  ContextConfig.acc_SampleRate = InAudioDevice->GetSampleRate();
  ContextConfig.acc_Size = sizeof(ovrAudioContextConfiguration);

  ovrAudioContext NewContext = nullptr;
  ovrResult Result = OVRA_CALL(ovrAudio_CreateContext)(&NewContext, &ContextConfig);

  if (ensure(Result == ovrSuccess)) {
    FScopeLock ScopeLock(&ContextCritSection);
    return ContextMap.Add(InAudioDevice, NewContext);
  } else {
    return nullptr;
  }
}

void FMetaXRAudioContextManager::DestroyContextForAudioDevice(const FAudioDevice* InAudioDevice) {
  FScopeLock ScopeLock(&ContextCritSection);
  ovrAudioContext* Context = ContextMap.Find(InAudioDevice);

  if (Context) {
    OVRA_CALL(ovrAudio_DestroyContext)(*Context);
    ContextMap.Remove(InAudioDevice);
  }
}
