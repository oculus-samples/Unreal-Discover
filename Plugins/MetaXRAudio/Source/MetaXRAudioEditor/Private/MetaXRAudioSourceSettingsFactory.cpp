// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetaXRAudioSourceSettingsFactory.h"
#include "AssetTypeCategories.h"
#include "AudioAnalytics.h"
#include "MetaXRAudioSourceSettings.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_MetaXRAudioSourceSettings::GetName() const {
  return LOCTEXT("AssetTypeActions_MetaXRAudioSpatializationSettings", "Meta XR Audio Source Settings");
}

const TArray<FText>& FAssetTypeActions_MetaXRAudioSourceSettings::GetSubMenus() const {
  static const TArray<FText> SubMenus{LOCTEXT("AssetSoundMetaXRSubMenu", "MetaXR")};

  return SubMenus;
}

FColor FAssetTypeActions_MetaXRAudioSourceSettings::GetTypeColor() const {
  return FColor(100, 100, 100);
}

UClass* FAssetTypeActions_MetaXRAudioSourceSettings::GetSupportedClass() const {
  return UMetaXRAudioSourceSettings::StaticClass();
}

uint32 FAssetTypeActions_MetaXRAudioSourceSettings::GetCategories() {
  return EAssetTypeCategories::Sounds;
}

UMetaXRAudioSourceSettingsFactory::UMetaXRAudioSourceSettingsFactory(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
  SupportedClass = UMetaXRAudioSourceSettings::StaticClass();

  bCreateNew = true;
  bEditorImport = true;
  bEditAfterNew = true;
}

UObject* UMetaXRAudioSourceSettingsFactory::FactoryCreateNew(
    UClass* Class,
    UObject* InParent,
    FName Name,
    EObjectFlags Flags,
    UObject* Context,
    FFeedbackContext* Warn) {
  Audio::Analytics::RecordEvent_Usage(TEXT("MetaXRAudio.SourceSettingsCreated"));
  return NewObject<UMetaXRAudioSourceSettings>(InParent, Name, Flags);
}

uint32 UMetaXRAudioSourceSettingsFactory::GetMenuCategories() const {
  return EAssetTypeCategories::Sounds;
}

#undef LOCTEXT_NAMESPACE
