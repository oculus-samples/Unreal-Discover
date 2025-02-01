// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AssetToolsModule.h"
#include "AssetTypeActions_Base.h"
#include "Factories/Factory.h"
#include "IAudioExtensionPlugin.h"
#include "MetaXRAmbisonicSettingsFactory.generated.h"

class FAssetTypeActions_MetaXRAmbisonicsSettings : public FAssetTypeActions_Base {
 public:
  virtual FText GetName() const override;
  virtual FColor GetTypeColor() const override;
  virtual const TArray<FText>& GetSubMenus() const override;
  virtual UClass* GetSupportedClass() const override;
  virtual uint32 GetCategories() override;
};

UCLASS(hidecategories = Object, MinimalAPI)
class UMetaXRAmbisonicsSettingsFactory : public UFactory {
  GENERATED_UCLASS_BODY()

  virtual UObject*
  FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

  virtual uint32 GetMenuCategories() const override;
};
