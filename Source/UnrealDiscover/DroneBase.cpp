// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "DroneBase.h"

ADroneBase::ADroneBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ADroneBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADroneBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float ADroneBase::PerlinNoise2D(const FVector2D& Location)
{
	return FMath::PerlinNoise2D(Location);
}
