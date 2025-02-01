// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetaXRAmbisonicSettingsFactory.h"
#include "AssetTypeCategories.h"
#include "AudioAnalytics.h"
#include "MetaXRAmbisonicsSettings.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_MetaXRAmbisonicsSettings::GetName() const {
  return LOCTEXT("AssetTypeActions_MetaXRAmbisonicsSettings", "MetaXR Ambisonics Settings");
}

FColor FAssetTypeActions_MetaXRAmbisonicsSettings::GetTypeColor() const {
  return FColor(100, 100, 100);
}

const TArray<FText>& FAssetTypeActions_MetaXRAmbisonicsSettings::GetSubMenus() const {
  static const TArray<FText> SubMenus{LOCTEXT("AssetSoundMetaXRSubMenu", "MetaXR")};

  return SubMenus;
}

UClass* FAssetTypeActions_MetaXRAmbisonicsSettings::GetSupportedClass() const {
  return UMetaXRAudioSoundfieldSettings::StaticClass();
}

uint32 FAssetTypeActions_MetaXRAmbisonicsSettings::GetCategories() {
  return EAssetTypeCategories::Sounds;
}

UMetaXRAmbisonicsSettingsFactory::UMetaXRAmbisonicsSettingsFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
  SupportedClass = UMetaXRAudioSoundfieldSettings::StaticClass();

  bCreateNew = true;
  bEditorImport = true;
  bEditAfterNew = true;
}

UObject* UMetaXRAmbisonicsSettingsFactory::FactoryCreateNew(
    UClass* Class,
    UObject* InParent,
    FName Name,
    EObjectFlags Flags,
    UObject* Context,
    FFeedbackContext* Warn) {
  Audio::Analytics::RecordEvent_Usage(TEXT("MetaXRAudio.AmbisonicsSettingsCreated"));
  return NewObject<UMetaXRAudioSoundfieldSettings>(InParent, Name, Flags);
}

uint32 UMetaXRAmbisonicsSettingsFactory::GetMenuCategories() const {
  return EAssetTypeCategories::Sounds;
}
#undef LOCTEXT_NAMESPACE
