// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Widgets/Input/SButton.h"
#include "Input/Reply.h"

#include "HapticButton.generated.h"


DECLARE_DELEGATE_OneParam(FHapticButtonInteract, uint32);

/**
 * Button with haptic feedback on click and hovered, with mappings from Pointer Index to EControllerHand
 */
UCLASS()
class UNREALDISCOVER_API UHapticButton : public UButton
{
	GENERATED_BODY()

public:
	UHapticButton(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptic Feedback")
	float HoverIntensity = .25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptic Feedback")
	float ClickIntensity = .75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptic Feedback")
	float Duration = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptic Feedback")
	TMap<int32, EControllerHand> InputIndexToControllerMap;

protected:
	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	//~ End UWidget Interface
	UFUNCTION()
	void HapticHandleClicked(uint32 PointerIndex);
	UFUNCTION()
	void HapticHandleHovered(uint32 PointerIndex);

	void PlayHaptic(float Intensity, EControllerHand Handedness);

	UFUNCTION()
	void StopHaptic(EControllerHand Handedness);

private:
	FTimerHandle HapticTimerHandleLeft;
	FTimerHandle HapticTimerHandleRight;
};

/**
 * Slate button with custom delegates
 */
class SHapticButton : public SButton
{
public:
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	FHapticButtonInteract OnHapticClick;
	FHapticButtonInteract OnHapticHover;
};
