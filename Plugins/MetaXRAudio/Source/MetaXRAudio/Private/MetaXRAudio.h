// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IAudioExtensionPlugin.h"
#include "MetaXRAmbisonicSpatializer.h"
#include "MetaXRAudioMixer.h"
#include "MetaXRAudioSourceSettings.h"

/************************************************************************/
/* FMetaXRSpatializationPluginFactory                                   */
/* Handles initialization of the required Meta XR Audio Spatialization   */
/* plugin.                                                              */
/************************************************************************/
class FMetaXRSpatializationPluginFactory : public IAudioSpatializationFactory {
 public:
  //~ Begin IAudioSpatializationFactory
  virtual FString GetDisplayName() override {
    static FString DisplayName = FString(TEXT("Meta XR Audio"));
    return DisplayName;
  }

  virtual bool SupportsPlatform(const FString& PlatformName) override {
    return PlatformName == FString(TEXT("Windows")) || PlatformName == FString(TEXT("Android"));
  }

  virtual UClass* GetCustomSpatializationSettingsClass() const override {
    return UMetaXRAudioSourceSettings::StaticClass();
  }

  virtual TAudioSpatializationPtr CreateNewSpatializationPlugin(FAudioDevice* OwningDevice) override;
  //~ End IAudioSpatializationFactory
};

class FMetaXRReverbPluginFactory : public IAudioReverbFactory {
 public:
  //~ Begin IAudioReverbFactory
  virtual FString GetDisplayName() override {
    static FString DisplayName = FString(TEXT("Meta XR Audio"));
    return DisplayName;
  }

  virtual bool SupportsPlatform(const FString& PlatformName) override {
    return PlatformName == FString(TEXT("Windows")) || PlatformName == FString(TEXT("Android"));
  }

  virtual TAudioReverbPtr CreateNewReverbPlugin(FAudioDevice* OwningDevice) override;
  //~ End IAudioReverbFactory
};
