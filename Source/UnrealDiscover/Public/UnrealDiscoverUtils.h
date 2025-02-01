// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UnrealDiscoverUtils.generated.h"


UCLASS()
class UNREALDISCOVER_API UUnrealDiscoverUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "UnrealDiscover|Utils")
	static const bool IsSimulatorActive();
};
