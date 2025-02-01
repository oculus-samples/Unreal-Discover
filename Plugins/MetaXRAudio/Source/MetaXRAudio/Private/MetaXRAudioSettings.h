// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MetaXRAudioSettings.generated.h"

UCLASS(config = Engine, defaultconfig, BlueprintType)
class METAXRAUDIO_API UMetaXRAudioSettings : public UObject {
  GENERATED_BODY()

 public:
  UMetaXRAudioSettings();

#if WITH_EDITOR
  virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
  virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif

  // The submix the Reverb will output to. If not set, a default submix will be used.
  UPROPERTY(GlobalConfig, EditAnywhere, Category = "Submix", meta = (AllowedClasses = "/Script/Engine.SoundSubmix"))
  FSoftObjectPath OutputSubmix;

  // Max number of spatialized voices. Must be larger than the total number of spatialized sounds that can play concurrently
  UPROPERTY(
      GlobalConfig,
      BlueprintReadOnly,
      EditAnywhere,
      Category = "Configuration",
      meta = (ClampMin = "1.0", ClampMax = "1024.0", UIMin = "1.0", UIMax = "1024.0"))
  int VoiceLimit;
};
