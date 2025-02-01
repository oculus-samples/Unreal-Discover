// Copyright (c) Meta Platforms, Inc. and affiliates.


#include "DiscoverInteractionComponent.h"

#include "Widgets/SWidget.h"

void UDiscoverInteractionComponent::PressPointerKey(FKey Key)
{
	Super::PressPointerKey(Key);

	if (bIsHoveredWidgetInteractable)
	{
		OnInteractableClicked.Broadcast();
	}
}

void UDiscoverInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsOverInteractableWidget())
	{
		FWidgetPath WidgetPath = LastWidgetPath.ToWidgetPath();
		for (FArrangedWidget& ArrangedWidget : WidgetPath.Widgets.GetInternalArray())
		{
			const TSharedRef<SWidget>& Widget = ArrangedWidget.Widget;
			if (Widget->IsHovered() && Widget->IsInteractable() && LastHoveredWidget != Widget)
			{
				OnInteractableHovered.Broadcast();
				LastHoveredWidget = Widget;
				break;
			}
		}
	}
	else
	{
		LastHoveredWidget = nullptr;
	}
}
