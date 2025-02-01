// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "MetaXRDiscoverUtilities.h"

#include "Kismet/GameplayStatics.h"
#include "Components/WidgetInteractionComponent.h"

#include <Interfaces/OnlineIdentityInterface.h>

DEFINE_LOG_CATEGORY(LogMetaXRDiscover);

void UMetaXRDiscoverUtilities::SystemLog(const FString& Message)
{
	UE_LOG(LogMetaXRDiscover, Display, TEXT("%s"), *Message);
}

EOculusXRControllerDrivenHandPoseTypes UMetaXRDiscoverUtilities::CurrentControllerDrivenHandPoseType = EOculusXRControllerDrivenHandPoseTypes::None;

EOculusXRControllerDrivenHandPoseTypes UMetaXRDiscoverUtilities::GetDiscoverControllerDrivenHandType()
{
	return CurrentControllerDrivenHandPoseType;
}

bool UMetaXRDiscoverUtilities::IsPIE()
{
#if WITH_EDITOR
	return true;
#else
	return false;
#endif
}

FString UMetaXRDiscoverUtilities::GetProjectVersion()
{
	FString Result;
	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("ProjectVersion"),
		Result,
		GGameIni
	);

	return Result;
}

EControllerHand UMetaXRDiscoverUtilities::GetControllerHoveringWidget(UObject* WorldContextObject, const UWidget* Widget, bool& OutIsHovered)
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(WorldContextObject, 0);
	TArray<UWidgetInteractionComponent*> InteractionComponents;
	PlayerPawn->GetComponents<UWidgetInteractionComponent>(InteractionComponents);


	EControllerHand Hand = EControllerHand();
	for (UWidgetInteractionComponent* InteractionComponent : InteractionComponents)
	{
		const FWeakWidgetPath& WidgetPath = InteractionComponent->GetHoveredWidgetPath();
		OutIsHovered = WidgetPath.ContainsWidget(Widget->GetCachedWidget().Get());
		if (OutIsHovered)
		{
			Hand = static_cast<EControllerHand>(InteractionComponent->PointerIndex);
			break;
		}
	}

	return Hand;
}

void UMetaXRDiscoverUtilities::SetDiscoverControllerDrivenHandType(EOculusXRControllerDrivenHandPoseTypes Type)
{
	CurrentControllerDrivenHandPoseType = Type;
	UOculusXRInputFunctionLibrary::SetControllerDrivenHandPoses(Type);
}

FString UMetaXRDiscoverUtilities::GenerateSessionName()
{
	FString Name(TEXT("ABCDEF"));
	for (auto& Ch : Name)
	{
		Ch = FMath::RandRange('A', 'Z');
	}
	return Name;
}
