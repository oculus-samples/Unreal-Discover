// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "IMetaXRAudioEditorPlugin.h"
#include "MetaXRAudioPlatform.h"

#include "MetaXRAudioRoomAcousticProperties.h"
#include "MetaXRAudioRoomAcousticVisualizer.h"


#ifdef META_NATIVE_UNREAL_PLUGIN
#include "MetaXRAmbisonicSettingsFactory.h"
#include "MetaXRAudioSettings.h"
#include "MetaXRAudioSourceSettingsFactory.h"
#endif // META_NATIVE_UNREAL_PLUGIN

#include "DynamicMeshBuilder.h"
#include "UnrealEd.h"

#include "Editor.h"
#include "Editor/UnrealEdEngine.h"
#include "EditorModeManager.h"
#include "EditorViewportClient.h"
#include "FrameTypes.h"
#include "ISettingsModule.h"
#include "LevelEditor.h"
#include "MaterialShared.h"
#include "Modules/ModuleManager.h"
#include "NavigationSystem.h"
#include "UnrealEdGlobals.h"

DECLARE_DELEGATE_RetVal_OneParam(TSharedRef<FExtender>, FShowMenuExtenderDelegate, const TSharedRef<FUICommandList>);

void FMetaXRAudioEditorPlugin::StartupModule() {
#ifdef META_NATIVE_UNREAL_PLUGIN
  // Register asset types
  IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
  {
    AssetTools.RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_MetaXRAudioSourceSettings));
    // At this time the ambisonic module has no settings so don't allow users to create them
    // However, it is required for it to exist so the ambisonic virtual functions can be fufilled
    // AssetTools.RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_MetaXRAmbisonicsSettings));

    ISettingsModule* SettingsModule = FModuleManager::Get().GetModulePtr<ISettingsModule>("Settings");
    if (SettingsModule) {
      SettingsModule->RegisterSettings(
          "Project",
          "Plugins",
          "Meta XR Audio",
          NSLOCTEXT("MetaXRAudio", "Meta XR Audio", "Meta XR Audio"),
          NSLOCTEXT("MetaXRAudio", "Configure Meta XR Audio settings", "Configure Meta XR Audio settings"),
          GetMutableDefault<UMetaXRAudioSettings>());
    }

  }
#endif

  // Register Visualizers
  if (GUnrealEd) {
    TSharedPtr<FMetaXRAudioRoomAcousticVisualizer> RoomAcousticVisualizer = MakeShareable(new FMetaXRAudioRoomAcousticVisualizer());
    GUnrealEd->RegisterComponentVisualizer(UMetaXRAudioRoomAcousticProperties::StaticClass()->GetFName(), RoomAcousticVisualizer);
    RoomAcousticVisualizer->OnRegister();
  }

  IConsoleManager::Get().RegisterConsoleVariable(
      TEXT("MetaXRAudioGizmos"),
      1,
      TEXT("Shows or hide Gizmos for MetaXR Audio Plugin\n") TEXT("<=0: Hide\n") TEXT("  1: Show\n"),
      ECVF_Default);

}

void FMetaXRAudioEditorPlugin::ShutdownModule() {
  if (GUnrealEd) {
    GUnrealEd->UnregisterComponentVisualizer(UMetaXRAudioRoomAcousticProperties::StaticClass()->GetFName());
  }

}


IMPLEMENT_MODULE(FMetaXRAudioEditorPlugin, MetaXRAudioEditor)
