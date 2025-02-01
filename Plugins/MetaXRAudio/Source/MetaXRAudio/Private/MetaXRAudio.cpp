// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetaXRAudio.h"
#include "Features/IModularFeatures.h"
#include "IMetaXRAudioPlugin.h"
#include "MetaXRAmbisonicSpatializer.h"
#include "MetaXRAudioReverb.h"

TAudioSpatializationPtr FMetaXRSpatializationPluginFactory::CreateNewSpatializationPlugin(FAudioDevice* OwningDevice) {
  FMetaXRAudioPlugin* Plugin = &FModuleManager::GetModuleChecked<FMetaXRAudioPlugin>("MetaXRAudio");
  check(Plugin != nullptr);
  Plugin->RegisterAudioDevice(OwningDevice);

  return TAudioSpatializationPtr(new MetaXRAudioSpatializationAudioMixer());
}

TAudioReverbPtr FMetaXRReverbPluginFactory::CreateNewReverbPlugin(FAudioDevice* OwningDevice) {
  FMetaXRAudioPlugin* Plugin = &FModuleManager::GetModuleChecked<FMetaXRAudioPlugin>("MetaXRAudio");
  check(Plugin != nullptr);
  Plugin->RegisterAudioDevice(OwningDevice);

  return TAudioReverbPtr(new MetaXRAudioReverb());
}
