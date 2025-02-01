// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
    public class MetaXRAudioEditor : ModuleRules
    {
        public MetaXRAudioEditor(ReadOnlyTargetRules Target) : base(Target)
        {
            PrivateIncludePaths.AddRange(
                new string[] {
                    "MetaXRAudioEditor/Public",
                    "MetaXRAudio/Private",
                    "MetaXRAudio/Public",
                    "MetaXRAudio/Private/LibMetaXRAudio/include",
                     // ... add other private include paths required here ...
                }
                );

            PrivateIncludePathModuleNames.AddRange(
            new string[] {
                "AssetTools",
                "AudioEditor",
                "MetaXRAudio",
                "UnrealEd",
                "RenderCore",
                }
                );

            PublicDependencyModuleNames.AddRange(
                new string[] {
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "InputCore",
                    "RenderCore",
                    "EditorFramework",
                    "UnrealEd",
                    "RHI",
                    "AudioEditor",
                    "AudioMixer",
                    "MetaXRAudio",
                }
            );
        }
    }
}
