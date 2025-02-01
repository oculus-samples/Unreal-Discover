// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;

namespace UnrealBuildTool.Rules
{
    public class MetaXRAudio : ModuleRules
    {
        public MetaXRAudio(ReadOnlyTargetRules Target) : base(Target)
        {
            PrivateIncludePathModuleNames.AddRange(
                new string[]
                {
                    "TargetPlatform"
                }
                );

            PrivateIncludePaths.AddRange(
                new string[] {
                    "MetaXRAudio/Private",
                    "MetaXRAudio/Private/LibMetaXRAudio/include",
                    System.IO.Path.Combine(GetModuleDirectory("AudioMixer"), "Private"),

                    // ... add other private include paths required here ...
                }
                );

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "AudioExtensions",
                    "AudioMixer",
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "Landscape",
                    "Projects",
                    "SignalProcessing",
                    "SoundFieldRendering",
                    "UMG",
                    "Json"
                }
                );

            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                // Automatically copy DLL to packaged builds
                RuntimeDependencies.Add(Path.Combine(PluginDirectory, "Binaries", "Win64", "metaxraudio64.dll"));
                AddEngineThirdPartyPrivateStaticDependencies(Target, "DX11Audio");
            }

            if (Target.Platform == UnrealTargetPlatform.Android)
            {
                // AndroidPlugin
                {
                    AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginDirectory, "Source", "MetaXRAudio", "MetaXRAudio_APL.xml"));
                    PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Source", "MetaXRAudio", "Private", "LibMetaXRAudio", "lib", "arm64-v8a", "libmetaxraudio64.so"));
                }
            }
        }
    }
}
