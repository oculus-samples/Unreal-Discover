// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetInteractionComponent.h"
#include "DiscoverInteractionComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDiscoverInteractableEvent);

/**
 * Custom Interaction Component to delegate events when hovering and clicking an interactable widget
 * 
 * Used to add haptic feedback to controllers
 */
UCLASS(ClassGroup = "UserInterface", meta = (BlueprintSpawnableComponent), MinimalAPI)
class UDiscoverInteractionComponent : public UWidgetInteractionComponent
{
	GENERATED_BODY()

public:
	virtual void PressPointerKey(FKey Key) override final;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override final;


	UPROPERTY(BlueprintAssignable, Category = "Discover WIC")
	FDiscoverInteractableEvent OnInteractableClicked;

	UPROPERTY(BlueprintAssignable, Category = "Discover WIC")
	FDiscoverInteractableEvent OnInteractableHovered;

private:
	TWeakPtr<SWidget> LastHoveredWidget = nullptr;
};
