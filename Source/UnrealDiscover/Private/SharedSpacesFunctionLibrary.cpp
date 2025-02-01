// Copyright (c) Meta Platforms, Inc. and affiliates.


#include "SharedSpacesFunctionLibrary.h"
#include "Serialization/JsonReader.h"
#include <stdio.h>


DEFINE_LOG_CATEGORY(LogSharedSpaces);

namespace SSFL
{
	const FString IsLobbyKey(TEXT("is_lobby"));
	const FString MapKey(TEXT("map"));
	const FString PublicRoomNameKey(TEXT("public_room_name"));
	const FString RoomIdKey(TEXT("room"));
}

FString USharedSpacesFunctionLibrary::AddQuotationMarks(const FString& DeepLink)
{
	constexpr TCHAR QuotationMark = '"';

	int32 Index = -1;
	FString Result;

	if (DeepLink.FindChar(QuotationMark, Index))
	{
		Result = DeepLink;
	}
	else
	{
		FString Left;
		FString Right;
		if (DeepLink.Contains(TEXT("Lobby")) || DeepLink.Contains(TEXT("PurpleRoom")))
		{
			DeepLink.Split(TEXT(","), &Left, &Right);

			FString Left2;
			FString Right2;

			Left.Split(TEXT(":"), &Left2, &Right2);
			Left2.InsertAt(1, QuotationMark);
			Left2.AppendChar(QuotationMark);
			Right2.InsertAt(0, QuotationMark);
			Right2.AppendChar(QuotationMark);
			Result.Append(Left2);
			Result.Append(":");
			Result.Append(Right2);

			Right.Split(TEXT(":"), &Left2, &Right2);
			Left2.InsertAt(0, QuotationMark);
			Left2.AppendChar(QuotationMark);
			Right2.InsertAt(0, QuotationMark);
			Right2.InsertAt(Right2.Len() - 1, QuotationMark);
			Result.Append(",");
			Result.Append(Left2);
			Result.Append(":");
			Result.Append(Right2);
		}
		else
		{
			DeepLink.Split(TEXT(":"), &Left, &Right);

			Left.InsertAt(1, QuotationMark);
			Left.AppendChar(QuotationMark);

			Right.InsertAt(0, QuotationMark);
			Right.InsertAt(Right.Len() - 1, QuotationMark);

			Result.Append(Left);
			Result.Append(":");
			Result.Append(Right);
		}
	}

	return Result;
}

void USharedSpacesFunctionLibrary::GetIsLobby(const FString& DeepLink, bool& bIsLobby)
{
	TSharedPtr<FJsonObject> DeepLinkJson = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(DeepLink);
	
	bIsLobby = false;
	FString IsLobbyValue;

	if (FJsonSerializer::Deserialize(JsonReader, DeepLinkJson) &&
		DeepLinkJson->TryGetStringField(SSFL::IsLobbyKey, IsLobbyValue))
	{
		bIsLobby = IsLobbyValue.Equals(TEXT("true"), ESearchCase::IgnoreCase);
	}
}

void USharedSpacesFunctionLibrary::GetLaunchMap(const FString& DeepLink, const FString& OptionalDefaultMapName, bool& HasMapName, FString& MapName)
{
	TSharedPtr<FJsonObject> DeepLinkJson = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(DeepLink);
	
	HasMapName =
		FJsonSerializer::Deserialize(JsonReader, DeepLinkJson) &&
		DeepLinkJson->TryGetStringField(SSFL::MapKey, MapName);

	if (!HasMapName && !OptionalDefaultMapName.IsEmpty())
	{
		MapName = OptionalDefaultMapName;
		HasMapName = true;
	}
}

void USharedSpacesFunctionLibrary::GetPublicRoomName(const FString& DeepLink, bool& HasPublicRoomName, FString& PublicRoomName)
{
	PublicRoomName.Empty();

	TSharedPtr<FJsonObject> DeepLinkJson = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(DeepLink);
	
	HasPublicRoomName =
		FJsonSerializer::Deserialize(JsonReader, DeepLinkJson) &&
		DeepLinkJson->TryGetStringField(SSFL::PublicRoomNameKey, PublicRoomName);
}

FString USharedSpacesFunctionLibrary::GetLocalCharacterLocationAndRotation()
{
	APlayerController* PlayerController = GWorld->GetFirstPlayerController();
	if (!PlayerController) return TEXT("");

	APawn* Pawn = PlayerController->GetPawn();
	if (!Pawn) return TEXT("");

	const FVector& Loc = Pawn->GetActorLocation();
	const FRotator& Rot = Pawn->GetActorRotation();

	return FString::Printf(
		TEXT("X%+0.2fY%+0.2fZ%+0.2fP%+0.2fY%+0.2fR%+0.2f"),
		Loc.X, Loc.Y, Loc.Z,
		Rot.Pitch, Rot.Yaw, Rot.Roll);
}

bool USharedSpacesFunctionLibrary::ParseLocationAndRotation(const FString& LocationAndRotationString, FVector& Location, FRotator& Rotation)
{
	float X, Y, Z, Pitch, Yaw, Roll;

#if PLATFORM_WINDOWS
	int ParsedItems = sscanf_s(TCHAR_TO_ANSI(*LocationAndRotationString), "X%fY%fZ%fP%fY%fR%f", &X, &Y, &Z, &Pitch, &Yaw, &Roll);
#else
	int ParsedItems = sscanf(TCHAR_TO_ANSI(*LocationAndRotationString), "X%fY%fZ%fP%fY%fR%f", &X, &Y, &Z, &Pitch, &Yaw, &Roll);
#endif


	if (ParsedItems == 6)
	{
		Location = FVector(X, Y, Z);
		Rotation = FRotator(Pitch, Yaw, Roll);
		return true;
	}

	return false;
}

void USharedSpacesFunctionLibrary::SystemLog(const FString& Message)
{
	UE_LOG(LogSharedSpaces, Display, TEXT("%s"), *Message);
}