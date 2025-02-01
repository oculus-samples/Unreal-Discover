// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MetaXRAudioPlatform.h"
#ifdef META_NATIVE_UNREAL_PLUGIN
#include "MetaXRAmbisonicSpatializer.h"
#include "MetaXRAudio.h"
#endif
#include "MetaXRAudioDllManager.h"
#include "Modules/ModuleManager.h"

/************************************************************************/
/* FMetaXRAudioPlugin                                                   */
/* Module interface. Also handles loading and unloading the Meta XR Audio*/
/* DLL using FMetaXRAudioDllManager.                                    */
/************************************************************************/
class FMetaXRAudioPlugin : public IModuleInterface {
 public:
  //~ Begin IModuleInterface
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;
  //~ End IModuleInterface
#ifdef META_NATIVE_UNREAL_PLUGIN
  void RegisterAudioDevice(FAudioDevice* AudioDeviceHandle);
  void UnregisterAudioDevice(FAudioDevice* AudioDeviceHandle);

  FMetaXRSpatializationPluginFactory* GetSpatializationPluginFactory() {
    return &PluginFactory;
  }
  FMetaXRReverbPluginFactory* GetReverbPluginFactory() {
    return &ReverbPluginFactory;
  }

 private:
  TArray<FAudioDevice*> RegisteredAudioDevices;
  FMetaXRSpatializationPluginFactory PluginFactory;
  FMetaXRReverbPluginFactory ReverbPluginFactory;
  FMetaXRAmbisonicsFactory AmbisonicsFactory;
#endif
};
