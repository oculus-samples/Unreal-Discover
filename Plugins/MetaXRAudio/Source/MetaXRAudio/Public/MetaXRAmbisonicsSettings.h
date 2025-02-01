// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IAudioExtensionPlugin.h"

#include "MetaXRAmbisonicsSettings.generated.h"

class FMetaXRAudioSoundfieldSettingsProxy : public ISoundfieldEncodingSettingsProxy {
 public:
  FMetaXRAudioSoundfieldSettingsProxy() {}

  virtual uint32 GetUniqueId() const override {
    // Currently there only one ID because there are no settings for ambisonics currently
    return (uint32)0;
  }

  virtual TUniquePtr<ISoundfieldEncodingSettingsProxy> Duplicate() const override {
    return TUniquePtr<ISoundfieldEncodingSettingsProxy>(new FMetaXRAudioSoundfieldSettingsProxy());
  }
};

UCLASS()
class METAXRAUDIO_API UMetaXRAudioSoundfieldSettings : public USoundfieldEncodingSettingsBase {
  GENERATED_BODY()

 public:
  virtual TUniquePtr<ISoundfieldEncodingSettingsProxy> GetProxy() const override {
    return TUniquePtr<ISoundfieldEncodingSettingsProxy>(new FMetaXRAudioSoundfieldSettingsProxy());
  }
};
