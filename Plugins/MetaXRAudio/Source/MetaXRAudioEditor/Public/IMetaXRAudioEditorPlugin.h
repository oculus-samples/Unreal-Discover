// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FMetaXRAudioEditorPlugin : public IModuleInterface {
 public:
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;
};
