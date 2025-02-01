// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DroneBase.generated.h"

UCLASS()
class UNREALDISCOVER_API ADroneBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADroneBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	float PerlinNoise2D(const FVector2D& Location);
};
