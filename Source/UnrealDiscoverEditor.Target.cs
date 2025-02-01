// Copyright (c) Meta Platforms, Inc. and affiliates.

using UnrealBuildTool;

public class UnrealDiscoverEditorTarget : TargetRules
{
	public UnrealDiscoverEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		BuildEnvironment = TargetBuildEnvironment.Unique;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;

		ExtraModuleNames.AddRange( new string[] { "UnrealDiscover" } );
	}
}
