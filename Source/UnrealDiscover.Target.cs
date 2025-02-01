// Copyright (c) Meta Platforms, Inc. and affiliates.

using UnrealBuildTool;

public class UnrealDiscoverTarget : TargetRules
{
	public UnrealDiscoverTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		bUseLoggingInShipping = true;
		BuildEnvironment = TargetBuildEnvironment.Unique;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;

		ExtraModuleNames.AddRange( new string[] { "UnrealDiscover" } );
	}
}
