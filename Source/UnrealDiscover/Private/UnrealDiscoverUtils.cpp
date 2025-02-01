// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "UnrealDiscoverUtils.h"
#include "OculusXRHMDRuntimeSettings.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformMisc.h"
#endif

FString GetSimulatorJsonPath()
{
	return GetMutableDefault<UOculusXRHMDRuntimeSettings>()->MetaXRJsonPath.FilePath;
}

// gets whether the simulator is active.
const bool UUnrealDiscoverUtils::IsSimulatorActive()
{
	// Unfortunately, OculusXRHMDModule is internal to the OculusXRHMD module.
	// This is a hacky work around to check if we are in the simulator...
#if PLATFORM_WINDOWS
	static const FString OpenXrRuntimeEnvKey = "XR_RUNTIME_JSON";
	FString MetaXRSimPath = GetSimulatorJsonPath();
	FString CurRuntimePath = FWindowsPlatformMisc::GetEnvironmentVariable(*OpenXrRuntimeEnvKey);
	return (!MetaXRSimPath.IsEmpty() && MetaXRSimPath == CurRuntimePath);
#else
	return false;
#endif
}
