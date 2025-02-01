// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "UnrealDiscover.h"
#include "Modules/ModuleManager.h"
#include "OculusDeveloperTelemetry.h"

OCULUS_TELEMETRY_LOAD_MODULE("Unreal-Discover");

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, UnrealDiscover, "UnrealDiscover");
