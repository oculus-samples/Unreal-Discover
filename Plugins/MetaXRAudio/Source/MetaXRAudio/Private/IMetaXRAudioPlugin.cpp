// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "IMetaXRAudioPlugin.h"
#include "AudioDevice.h"
#include "Features/IModularFeatures.h"
#include "MetaXRAudioPlatform.h"
#ifdef META_NATIVE_UNREAL_PLUGIN
#include "MetaXRAudioContextManager.h"
#endif

void FMetaXRAudioPlugin::StartupModule() {
  FMetaXRAudioLibraryManager::Get().Initialize();
#ifdef META_NATIVE_UNREAL_PLUGIN
  IModularFeatures::Get().RegisterModularFeature(FMetaXRSpatializationPluginFactory::GetModularFeatureName(), &PluginFactory);
  IModularFeatures::Get().RegisterModularFeature(FMetaXRReverbPluginFactory::GetModularFeatureName(), &ReverbPluginFactory);
#endif
};

void FMetaXRAudioPlugin::ShutdownModule() {
  FMetaXRAudioLibraryManager::Get().Shutdown();
}

#ifdef META_NATIVE_UNREAL_PLUGIN
void FMetaXRAudioPlugin::RegisterAudioDevice(FAudioDevice* AudioDeviceHandle) {
  // Inject the Context into the spatailizer (and reverb) if they're enabled
  if (!AudioDeviceHandle->bAudioMixerModuleLoaded) {
    return; // Not supported in old audio engine
  }

  if (!RegisteredAudioDevices.Contains(AudioDeviceHandle)) {
    TAudioPluginListenerPtr ContextManager = TAudioPluginListenerPtr(new FMetaXRAudioContextManager());
    AudioDeviceHandle->RegisterPluginListener(ContextManager);
    RegisteredAudioDevices.Add(AudioDeviceHandle);
  }
}

void FMetaXRAudioPlugin::UnregisterAudioDevice(FAudioDevice* AudioDeviceHandle) {
  RegisteredAudioDevices.Remove(AudioDeviceHandle);
}
#endif

IMPLEMENT_MODULE(FMetaXRAudioPlugin, MetaXRAudio)
