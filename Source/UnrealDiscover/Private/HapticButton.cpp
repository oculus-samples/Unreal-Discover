// Copyright (c) Meta Platforms, Inc. and affiliates.


#include "HapticButton.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Components/ButtonSlot.h"
#include "TimerManager.h"

#include "MetaXRDiscoverUtilities.h"

UHapticButton::UHapticButton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InputIndexToControllerMap.Add(0, EControllerHand::Left);
	InputIndexToControllerMap.Add(1, EControllerHand::Right);
}

TSharedRef<SWidget> UHapticButton::RebuildWidget()
{
	// Same function as parent, but with SHapticButton
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	MyButton = SNew(SHapticButton)
		.OnClicked(BIND_UOBJECT_DELEGATE(FOnClicked, SlateHandleClicked))
		.OnPressed(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandlePressed))
		.OnReleased(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleReleased))
		.OnHovered_UObject(this, &ThisClass::SlateHandleHovered)
		.OnUnhovered_UObject(this, &ThisClass::SlateHandleUnhovered)
		.ButtonStyle(&GetStyle())
		.ClickMethod(GetClickMethod())
		.TouchMethod(GetTouchMethod())
		.PressMethod(GetPressMethod())
		.IsFocusable(GetIsFocusable());
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	if (GetChildrenCount() > 0)
	{
		Cast<UButtonSlot>(GetContentSlot())->BuildSlot(MyButton.ToSharedRef());
	}


	// Bind function to delegates
	SHapticButton* HapticButton = static_cast<SHapticButton*>(MyButton.Get());
	HapticButton->OnHapticClick.BindUObject(this, &UHapticButton::HapticHandleClicked);
	HapticButton->OnHapticHover.BindUObject(this, &UHapticButton::HapticHandleHovered);

	return MyButton.ToSharedRef();
}

void UHapticButton::HapticHandleClicked(uint32 PointerIndex)
{
	PlayHaptic(ClickIntensity, InputIndexToControllerMap[PointerIndex]);
}

void UHapticButton::HapticHandleHovered(uint32 PointerIndex)
{
	PlayHaptic(HoverIntensity, InputIndexToControllerMap[PointerIndex]);
}

void UHapticButton::PlayHaptic(float Intensity, EControllerHand Handedness)
{
	// Play haptics
	UWorld* World = GetWorld();
	APlayerController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	Controller->SetHapticsByValue(1.0f, Intensity, Handedness);

	// Stop haptics after duration
	FTimerDelegate Delegate;
	Delegate.BindUObject(this, &UHapticButton::StopHaptic, Handedness);
	World->GetTimerManager().SetTimer(Handedness == EControllerHand::Left ? HapticTimerHandleLeft : HapticTimerHandleRight, Delegate, 1.0f, false, Duration);
}

void UHapticButton::StopHaptic(EControllerHand Handedness)
{
	APlayerController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	Controller->StopHapticEffect(Handedness);

	GetWorld()->GetTimerManager().ClearTimer(Handedness == EControllerHand::Left ? HapticTimerHandleLeft : HapticTimerHandleRight);
}

void SHapticButton::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// Do parent behavior
	this->SButton::OnMouseEnter(MyGeometry, MouseEvent);

	// Perform haptic feedback
	uint32 PointerIndex = MouseEvent.GetPointerIndex();
	OnHapticHover.ExecuteIfBound(PointerIndex);
}

FReply SHapticButton::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// Do parent behavior
	FReply Reply = this->SButton::OnMouseButtonDown(MyGeometry, MouseEvent);

	// Perform haptic feedback
	uint32 PointerIndex = MouseEvent.GetPointerIndex();
	OnHapticClick.ExecuteIfBound(PointerIndex);

	return Reply;
}
