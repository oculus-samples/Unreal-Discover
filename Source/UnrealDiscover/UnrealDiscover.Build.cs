// Copyright (c) Meta Platforms, Inc. and affiliates.

using UnrealBuildTool;

public class UnrealDiscover : ModuleRules
{
	public UnrealDiscover(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] 
			{
				"Core", 
				"CoreUObject",
				"Engine", 
				"InputCore",
				"XRBase",
				"OnlineSubsystem", 
				"OnlineSubsystemEOS",
				"OculusXRHMD",
				"OculusXRAnchors",
				"OculusXRScene",
				"OculusXRInput",
				"EnhancedInput",
				"Json",
				"OVRPlatform",
            });

		PrivateDependencyModuleNames.AddRange(new string[] 
			{
				"OnlineSubsystem",
				"SlateCore",
				"Slate",
				"UMG",
				"OnlineSubsystemUtils",
				"EOSVoiceChat",
				"OculusUtils",
			});
    }
}

