// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "Components/Widget.h"
#include "OculusXRSceneActor.h"
#include "OculusXRInputFunctionLibrary.h"


#include "MetaXRDiscoverUtilities.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMetaXRDiscover, Log, All);

/**
 * Utility functions for Unreal-Discover project
 */
UCLASS()
class UNREALDISCOVER_API UMetaXRDiscoverUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "MetaXR | Discover")
	static void SystemLog(const FString& Message);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MetaXR | Discover")
	static EOculusXRControllerDrivenHandPoseTypes GetDiscoverControllerDrivenHandType();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MetaXR | Discover")
	static bool IsPIE();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MetaXR | Discover")
	static FString GetProjectVersion();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MetaXR | Discover", meta = (WorldContext = "WorldContextObject"))
	static EControllerHand GetControllerHoveringWidget(UObject* WorldContextObject, const UWidget* Widget, bool& OutValid);

	UFUNCTION(BlueprintCallable, Category = "MetaXR | Discover")
	static void SetDiscoverControllerDrivenHandType(EOculusXRControllerDrivenHandPoseTypes Type);

	UFUNCTION(BlueprintPure, Category = "MetaXR | Discover")
	static FString GenerateSessionName();

private:
	static EOculusXRControllerDrivenHandPoseTypes CurrentControllerDrivenHandPoseType;
};
