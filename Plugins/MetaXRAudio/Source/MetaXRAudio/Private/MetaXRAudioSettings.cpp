// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetaXRAudioSettings.h"

UMetaXRAudioSettings::UMetaXRAudioSettings() : VoiceLimit(64) {}

#if WITH_EDITOR
void UMetaXRAudioSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
  // TODO notify

  Super::PostEditChangeProperty(PropertyChangedEvent);
}

bool UMetaXRAudioSettings::CanEditChange(const FProperty* InProperty) const {
  // TODO disable settings when reflection engine is disabled

  return Super::CanEditChange(InProperty);
}
#endif
