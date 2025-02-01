// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "EOSSubsystem.h"

#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystemTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/OutputDeviceNull.h"
#include "Online/OnlineSessionNames.h"
#include "Private/MetaXRDiscoverUtilities.h"
#include "VoiceChat.h"



DEFINE_LOG_CATEGORY(LogEOS);
const FName JoinabilitySessionSettingKey = "JOINABLE";
const FName LobbyNameQuerySettingKey = "LobbyName";



ESessionState ConvertInternalSessionState(const EOnlineSessionState::Type InternalSessionState)
{
	// because ESessionState is copied from EOnlineSessionState::Type (OnlineSubsystemTypes.h), 
	// we must be careful not to use any casting operations. IF EOnlineSessionState::Type changed in order, naming, count, etc...
	// we are better off getting compiler errors then having a bad GetSessionState method.
	switch (InternalSessionState)
	{
		case EOnlineSessionState::Type::NoSession:
			return ESessionState::NoSession;
		case EOnlineSessionState::Type::Creating:
			return ESessionState::Creating;
		case EOnlineSessionState::Type::Pending:
			return ESessionState::Pending;
		case EOnlineSessionState::Type::Starting:
			return ESessionState::Starting;
		case EOnlineSessionState::Type::InProgress:
			return ESessionState::InProgress;
		case EOnlineSessionState::Type::Ending:
			return ESessionState::Ending;
		case EOnlineSessionState::Type::Ended:
			return ESessionState::Ended;
		case EOnlineSessionState::Type::Destroying:
			return ESessionState::Destroying;
		default:
			return ESessionState::Invalid;
	}
}


const FUniqueNetId* UEOSSubsystem::GetLocalUniqueNetID(bool bOnlyIfLoggedIn) const
{
	if (!IdentityPtr)
		return nullptr;
	
	FUniqueNetIdPtr UNIDPtr = IdentityPtr->GetUniquePlayerId(0);
	if (!UNIDPtr->IsValid())
		return nullptr;

	if (bOnlyIfLoggedIn)
	{
		const FUniqueNetId& LocalUniqueNetIDPtr = *UNIDPtr;
		ELoginStatus::Type LoginStat = IdentityPtr->GetLoginStatus(LocalUniqueNetIDPtr);
		if (LoginStat == ELoginStatus::Type::LoggedIn)
			return UNIDPtr.Get();
		
		return nullptr;
	}
	
	return UNIDPtr.Get();
}


void UEOSSubsystem::Login()
{
	if (IdentityPtr)
	{
		FOnlineAccountCredentials OnlineCreds;
		OnlineCreds.Type = "deviceID";
		OnlineCreds.Id = "";
		OnlineCreds.Token = "";


		LogEntry(FString::Printf(TEXT("Attempting to login")));
		bool bLoginTaskStarted = IdentityPtr->Login(0, OnlineCreds);
		LogEntry(FString::Printf(TEXT("Login Task Started: %s"), bLoginTaskStarted ? TEXT("true") : TEXT("false")));
	}
}

void UEOSSubsystem::CreateSession(const FName& SessionName, const bool bWithVoiceChat)
{
	LogEntry("[UEOSSubsystem::CreateSession]");
	const ESessionState SessionState = GetSessionState();
	if (SessionState != ESessionState::NoSession)
	{
		LogError("[UEOSSubsystem] Unable to create session. IF we are in a state other than NoSession, the previous session should be destroyed before calling CreateSession", false);
		OnRoomCreated.Broadcast(false, false);
		return;
	}

	if (SessionPtr)
	{
		FOnlineSessionSettings SessionSettings{};
		SessionSettings.bAllowInvites = false;
		SessionSettings.bAllowJoinInProgress = true;
		SessionSettings.bAllowJoinViaPresence = false;
		SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
		SessionSettings.bIsDedicated = false;
		SessionSettings.bIsLANMatch = false;
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bUseLobbiesIfAvailable = true;
		SessionSettings.bUseLobbiesVoiceChatIfAvailable = bWithVoiceChat;
		SessionSettings.NumPublicConnections = 8;
		SessionSettings.SessionIdOverride = SessionName != NAME_None ? SessionName.ToString() : UMetaXRDiscoverUtilities::GenerateSessionName();
		SessionSettings.Set(SEARCH_LOBBIES, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		SessionSettings.Set(LobbyNameQuerySettingKey, SessionSettings.SessionIdOverride, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		LogEntry(FString::Printf(TEXT("Trying to create session with name = %s"), *SessionSettings.SessionIdOverride));
		SessionPtr->CreateSession(0, FName(SessionSettings.SessionIdOverride), SessionSettings);
	}
}

void UEOSSubsystem::StartSession()
{
	check(OnlineSubsystem != nullptr);
	check(SessionPtr != nullptr);
	const ESessionState SessionState = GetSessionState();
	const FName SessionName = GetCurrentSessionName();
	if (SessionState != ESessionState::Pending)
	{
		LogError("[UEOSSubsystem] Unable to start session. IF we are in a state other than ESessionState::Pending, the previous session should be Ended then Destroyed to start another session", false);
		OnSessionStarted.Broadcast(SessionName.ToString(), false);
		return;
	}

	SessionPtr->StartSession(SessionName);
}

void UEOSSubsystem::EndSession()
{
	check(OnlineSubsystem != nullptr);
	check(SessionPtr != nullptr);
	const ESessionState SessionState = GetSessionState();
	const FName SessionName = GetCurrentSessionName();
	if (SessionState != ESessionState::InProgress)
	{
		LogError("[UEOSSubsystem] Unable to End session. IF we are in a state other than ESessionState::Pending, the previous session should be Ended then Destroyed to start another session", false);
		FEOSOnEndSession.Broadcast(SessionName.ToString(), false);
		return;
	}

	SessionPtr->EndSession(SessionName);
}

void UEOSSubsystem::FindSession(const FName& SessionName)
{
	if (SessionPtr)
	{
		if (SessionSearch != nullptr)
			SessionSearch.Reset();

		SessionSearch = MakeShareable(new FOnlineSessionSearch);
		SessionSearch->bIsLanQuery = false;
		SessionSearch->MaxSearchResults = 1;
		SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
		SessionSearch->QuerySettings.Set(LobbyNameQuerySettingKey, SessionName.ToString(), EOnlineComparisonOp::Equals);
		SessionPtr->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UEOSSubsystem::JoinSession()
{
	if (SessionPtr)
	{
		SessionPtr->JoinSession(0, CurrentSessionName, SearchResult);
	}
}

void UEOSSubsystem::UpdateJoinability(const bool Joinable)
{
	check(OnlineSubsystem != nullptr);
	check(SessionPtr != nullptr);
	const FName CSessionName = GetCurrentSessionName();
	const FOnlineSessionSettings* SessionSettings = SessionPtr->GetSessionSettings(CSessionName);
	if (!SessionSettings)
		return;


	FOnlineSessionSettings NewSettings = *SessionSettings;
	NewSettings.bShouldAdvertise = Joinable;

	// add any custom settings
	FOnlineSessionSetting JoinabilitySessionSetting(Joinable, EOnlineDataAdvertisementType::Type::ViaOnlineServiceAndPing);
	NewSettings.Set(JoinabilitySessionSettingKey, JoinabilitySessionSetting);

	SessionPtr->UpdateSession(CSessionName, NewSettings);
}

void UEOSSubsystem::DestroySession()
{
	if (SessionPtr && !CurrentSessionName.IsNone())
	{
		LogEntry(FString::Printf(TEXT("Attempting to Destroy Session %s "), *CurrentSessionName.ToString()));

		if (bIsMasterClient)
		{
			OnMasterClientChanged.Broadcast();
		}

		SessionPtr->DestroySession(CurrentSessionName);
		CurrentSessionName = FName();
	}
}


ESessionState UEOSSubsystem::GetSessionState() const
{
	check(OnlineSubsystem != nullptr);
	check(SessionPtr != nullptr);
	const EOnlineSessionState::Type InternalSessionState = SessionPtr->GetSessionState(GetCurrentSessionName());
	ESessionState CurrentSessionState = ConvertInternalSessionState(InternalSessionState);
	
	if (CurrentSessionState == ESessionState::Invalid)
		UE_LOG(LogEOS, Error, TEXT("[UEOSSubsystem] Invalid Session State detected"));
	
	return CurrentSessionState;
}

FString UEOSSubsystem::GetNetID()
{
	if (IdentityPtr)
	{
		auto Result = IdentityPtr->GetUniquePlayerId(0).Get()->ToString();
		Result.RemoveAt(0);
		return Result;
	}
	return FString();
}

FString UEOSSubsystem::GetHostAddress()
{
	FString TravelURL;
	if (SessionPtr)
	{
		SessionPtr->GetResolvedConnectString(CurrentSessionName, TravelURL);
	}

	return TravelURL;
}


void UEOSSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{

	IOnlineSubsystem* OnlineSubsystemInterface = Online::GetSubsystem(GetWorld());
	if (!OnlineSubsystemInterface)
	{
		LogError("Unable to get the Online Subsystem Interface");
		NotifyGameInstanceThatEOSInitialized(false);
		return;
	}
	
	OnlineSubsystem = static_cast<IOnlineSubsystemEOS*>(OnlineSubsystemInterface);

	IdentityPtr = OnlineSubsystem->GetIdentityInterface();
	if (!IdentityPtr.IsValid())
	{
		LogError("Unable to get Online Identity Interface");
		NotifyGameInstanceThatEOSInitialized(false);
		return;
	}
	
	IdentityPtr->OnLoginCompleteDelegates->AddUObject(this, &UEOSSubsystem::OnLoginComplete);

	SessionPtr = OnlineSubsystem->GetSessionInterface();
	if (!SessionPtr.IsValid())
	{
		LogError("Unable to get Online Session Interface");
		NotifyGameInstanceThatEOSInitialized(false);
		return;
	}

	SessionPtr->OnCreateSessionCompleteDelegates.AddUObject(this, &UEOSSubsystem::OnCreateSessionComplete);
	SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOSSubsystem::OnFindSessionsComplete);
	SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOSSubsystem::OnJoinSessionComplete);
	SessionPtr->OnDestroySessionCompleteDelegates.AddUObject(this, &UEOSSubsystem::OnDestroySessionComplete);
	SessionPtr->OnSessionParticipantJoinedDelegates.AddUObject(this, &UEOSSubsystem::OnPlayerJoinedSession);
	SessionPtr->OnSessionParticipantLeftDelegates.AddUObject(this, &UEOSSubsystem::OnPlayerLeftSession);
	SessionPtr->OnStartSessionCompleteDelegates.AddUObject(this, &UEOSSubsystem::OnSessionStartedInternal);
	SessionPtr->OnEndSessionCompleteDelegates.AddUObject(this, &UEOSSubsystem::OnSessionEndedInternal);
	// session update delegates
	SessionPtr->OnSessionSettingsUpdatedDelegates.AddUObject(this, &UEOSSubsystem::OnSessionSettingsUpdatedInternal);
	SessionPtr->OnUpdateSessionCompleteDelegates.AddUObject(this, &UEOSSubsystem::OnUpdateSessionCompleteInternal);

	FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddUObject(this, &UEOSSubsystem::HandleApplicationEnterBackground);
	FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddUObject(this, &UEOSSubsystem::HandleApplicationEnterForeground);

	bIsMasterClient = false;
	bEnterForegroundFirstTime = true;
	BackgroundCounter = 0;

	NotifyGameInstanceThatEOSInitialized(true);
	
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("FindSession"),
		TEXT("Find an EOS session"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray< FString >& Params)
		{
			if (Params.Num() != 0)
			{
				FindSession(FName(Params[0]));
			}
		}));

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("JoinSession"),
		TEXT("Join an EOS session"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]
		{
			JoinSession();
		}));
}

void UEOSSubsystem::NotifyGameInstanceThatEOSInitialized(bool bEOSInit) const
{
	//The subsystem notifies the game instance when it is ready.
	FOutputDeviceNull OutputDeviceNull;
	GetGameInstance()->CallFunctionByNameWithArguments(
		*FString::Printf(TEXT("EOSInitialized %s"), bEOSInit ? TEXT("true") : TEXT("false")),
		OutputDeviceNull,
		nullptr,
		true);
}

void UEOSSubsystem::OnHostSuccess(UObject* WorldContextObject, FString LevelName)
{
	auto CurrentWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	auto NewURL = FURL(*LevelName);
	CurrentWorld->Listen(NewURL);

	LogEntry(FString("Started to listen to connections"));
}

void UEOSSubsystem::OnLoginComplete(int32 NumPlayers, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	if (bWasSuccessful)
	{
		LogEntry(FString::Printf(TEXT("Login Successful. UserID = %s"), *UserId.ToString()));
	}
	else
	{
		LogError(FString::Printf(TEXT("Login Failed. Reason = %s"), *Error));
	}
	OnLogin.Broadcast(bWasSuccessful);
};

void UEOSSubsystem::OnCreateSessionComplete(FName NewSessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		LogEntry(FString::Printf(TEXT("Session created. Session Name = %s"), *NewSessionName.ToString()));
		bIsMasterClient = true;
		CurrentSessionName = NewSessionName;
		BackgroundCounter = 0;
	}
	else
	{
		LogError(FString::Printf(TEXT("Session creation failed.")));
	}

	OnRoomCreated.Broadcast(bWasSuccessful, bIsMasterClient);
};

void UEOSSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	++BackgroundCounter;
	FString ErrMessage = "";
	const int32 SessionCount = SessionSearch->SearchResults.Num();
	if (bWasSuccessful && SessionCount > 0)
	{
		bool bFoundJoinableSession = false;
		ErrMessage = "Founds Sessions, but none joinable";
		for (const FOnlineSessionSearchResult& Lobby : SessionSearch->SearchResults)
		{
			LogEntry(FString::Printf(TEXT("Found session with id: %s"), *Lobby.GetSessionIdStr()));

			// A bit hacky here. We use a custom JOINABLE setting instead of relying on StartSession + bAllowJoinInProgress.
			// At the time of adding this, Calling StartSession while having bAllowJoinInProgress = false did not prevent players from joining.
			// Instead of debugging that, we use a custom session flag instead. IF JOINABLE NOT present THEN we assume its joinable.
			FOnlineSessionSettings SessionSettings = Lobby.Session.SessionSettings;
			bool IsJoinable;
			if (SessionSettings.Get<bool>(JoinabilitySessionSettingKey, IsJoinable) && !IsJoinable)
			{
				ErrMessage = "[UEOSSubsystem] Found a Session, but is NOT joinable.";
				LogEntry(ErrMessage, false);
			}
			else
			{
				SearchResult = Lobby;
				CurrentSessionName = FName(*Lobby.GetSessionIdStr());
				bFoundJoinableSession = true;
				ErrMessage.Empty();
				break;
			}
		}

		OnRoomFound.Broadcast(bFoundJoinableSession, ErrMessage);
	}
	else
	{
		ErrMessage = "[UEOSSubsystem] Did NOT find any sessions";
		LogError(ErrMessage);
		OnRoomFound.Broadcast(false, ErrMessage);
	}
}

void UEOSSubsystem::OnJoinSessionComplete(FName JoinedSessionName, EOnJoinSessionCompleteResult::Type Result)
{
	bool bSuccess = false;
	FString TravelURL;
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		SessionPtr->GetResolvedConnectString(JoinedSessionName, TravelURL);

		LogEntry(FString::Printf(TEXT("Session Joined = %s"), *JoinedSessionName.ToString()));
		LogEntry(FString::Printf(TEXT("TURL = %s"), *TravelURL));
		CurrentSessionName = JoinedSessionName;
		bSuccess = true;
		bIsMasterClient = false;
		BackgroundCounter = 0;
	}
	else if(Result == EOnJoinSessionCompleteResult::SessionIsFull)
	{
		LogError(FString::Printf(TEXT("Unable to join session. Reason: Session is full.")));
	}
	else if (Result == EOnJoinSessionCompleteResult::SessionDoesNotExist)
	{
		LogError(FString::Printf(TEXT("Unable to join session. Reason: Session doesn't exist.")));
	}
	else if (Result == EOnJoinSessionCompleteResult::CouldNotRetrieveAddress)
	{
		LogError(FString::Printf(TEXT("Unable to join session. Reason: Could not retrieve host address.")));
	}
	else if (Result == EOnJoinSessionCompleteResult::AlreadyInSession)
	{
		LogError(FString::Printf(TEXT("Unable to join session. Reason: User is already in the session.")));
	}
	else if (Result == EOnJoinSessionCompleteResult::UnknownError)
	{
		LogError(FString::Printf(TEXT("Unable to join session. Reason: Unknown error.")));
	}

	OnRoomJoined.Broadcast(bSuccess, TravelURL);
}

void UEOSSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (bIsMasterClient)
			bIsMasterClient = false;

		LogEntry(FString::Printf(TEXT("Destroyed Session %s"), *SessionName.ToString()));
	}

	OnRoomDestroyed.Broadcast(bWasSuccessful);
}

void UEOSSubsystem::OnPlayerJoinedSession(FName SessionName, const FUniqueNetId& PlayerJoinedID)
{
	LogEntry(FString::Printf(TEXT("Player %s joined the session %s"), *PlayerJoinedID.ToString(), *SessionName.ToString()));
	if (auto NamedSession = SessionPtr->GetNamedSession(SessionName))
	{
		if (NamedSession->bHosting && !bIsMasterClient)
		{
			bIsMasterClient = true;
		}
	}
	else
	{
		LogError(FString::Printf(TEXT("Failed to get named session. Can't check if the master client changed.")));
	}

	auto NetID = PlayerJoinedID.ToString();
	NetID.RemoveAt(0);
	OnPlayerJoined.Broadcast(NetID);
}

void UEOSSubsystem::OnPlayerLeftSession(FName SessionName, const FUniqueNetId& PlayerLeftID, EOnSessionParticipantLeftReason Reason)
{
	LogEntry(FString::Printf(TEXT("Player %s left the session %s"), *PlayerLeftID.ToString(), *SessionName.ToString()));
	auto NetID = PlayerLeftID.ToString();
	NetID.RemoveAt(0);
	OnPlayerLeft.Broadcast(NetID);
}

void UEOSSubsystem::OnSessionStartedInternal(FName SessionName, bool bWasSuccessful)
{
	if(!bWasSuccessful)
		LogError("[UEOSSubsystem] Unable to start current session", false);
	OnSessionStarted.Broadcast(SessionName.ToString(), bWasSuccessful);
}

void UEOSSubsystem::OnSessionEndedInternal(FName SessionName, bool bWasSuccessful)
{
	if (!bWasSuccessful)
		LogError("[UEOSSubsystem] Unable to end session", false);
	FEOSOnEndSession.Broadcast(SessionName.ToString(), bWasSuccessful);
}


void UEOSSubsystem::OnSessionSettingsUpdatedInternal(FName SessionName, const FOnlineSessionSettings& SessionSettings)
{
	auto IsAdvertisingStr = SessionSettings.bShouldAdvertise ? "IS advertising" : "NOT advertising";
	const FString Msg = FString::Printf(TEXT("[UEOSSubsystem] Updated Session Settings. New Advertisement value: %s"), IsAdvertisingStr);
	LogEntry(Msg, false);
}

void UEOSSubsystem::OnUpdateSessionCompleteInternal(FName SessionName, bool WasSuccessful)
{
	if (!WasSuccessful) {
		const FString ErrMsg = "[UEOSSubsystem] Unable to update session settings for session: " + SessionName.ToString();
		LogError(ErrMsg, false);
		return;
	}

	LogEntry("[UEOSSubsystem] Updated Session Complete!", false);
}


void UEOSSubsystem::LogEntry(const FString& LogEntry, bool bBroadcast)
{
	if (bBroadcast)
	{
		OnLogEntry.Broadcast(LogEntry);
	}
	UE_LOG(LogEOS, Display, TEXT("%s"), *LogEntry);
}

void UEOSSubsystem::LogWarning(const FString& LogEntry, bool bBroadcast)
{
	if (bBroadcast)
	{
		OnLogEntry.Broadcast(LogEntry);
	}

	UE_LOG(LogEOS, Warning, TEXT("%s"), *LogEntry);
}

void UEOSSubsystem::LogError(const FString& LogEntry, bool bBroadcast)
{
	if (bBroadcast)
	{
		OnLogEntry.Broadcast(LogEntry);
	}

	UE_LOG(LogEOS, Error, TEXT("%s"), *LogEntry);

}


void UEOSSubsystem::HandleApplicationEnterBackground()
{
	if (BackgroundCounter == 0)
	{
		++BackgroundCounter;
		LogEntry(TEXT("Application Entering Background/Quitting"));
		DestroySession();
		UGameplayStatics::OpenLevel(GetWorld(), "TransitionMap");
	}
}

void UEOSSubsystem::HandleApplicationEnterForeground()
{
	if (BackgroundCounter <= 1 && !bEnterForegroundFirstTime)
	{
		LogEntry(TEXT("Application Entering Foreground"));
		OnAppEntersForeground.Broadcast();
	}
	else
	{
		bEnterForegroundFirstTime = false;
	}
}



inline const bool IsLoggedInVoiceChatNoCheck(const IVoiceChatUser* UsersVoiceChatInterface)
{
	return UsersVoiceChatInterface->IsLoggedIn();
}

const bool UEOSSubsystem::IsInVoiceChat() const
{
	const FUniqueNetId* LocalUniqueNetID = GetLocalUniqueNetID(true);
	if (LocalUniqueNetID == nullptr)
		return false;

	const IVoiceChatUser* UsersVoiceChatInterface = OnlineSubsystem->GetVoiceChatUserInterface(*LocalUniqueNetID);
	return IsLoggedInVoiceChatNoCheck(UsersVoiceChatInterface);
}

// return value means the call was successful (we were able to set mute state).
const bool UEOSSubsystem::SetLocalPlayerVoiceChatMuteState(const bool bNewMuted)
{
	const FUniqueNetId* LocalUniqueNetID = GetLocalUniqueNetID(true);
	if (LocalUniqueNetID == nullptr)
	{
		UE_LOG(LogEOS, Warning, TEXT("Attempting to set local mute status of Voice chat without being logged in..."));
		return false;
	}

	IVoiceChatUser* UsersVoiceChatInterface = OnlineSubsystem->GetVoiceChatUserInterface(*LocalUniqueNetID);
	if (IsLoggedInVoiceChatNoCheck(UsersVoiceChatInterface))
	{
		UsersVoiceChatInterface->SetAudioInputDeviceMuted(bNewMuted);
		return true;
	}
	return false;
}

// return value means the call was successful (we were able to get the voice chat mute state).
// The actual mute state is bMuteState
const bool UEOSSubsystem::GetLocalPlayerVoiceChatMuteState(bool& bMuteState) const
{
	bMuteState = false;
	const FUniqueNetId* LocalUniqueNetID = GetLocalUniqueNetID(true);
	if (LocalUniqueNetID == nullptr)
	{
		UE_LOG(LogEOS, Warning, TEXT("Attempting to get local mute status of Voice chat without being logged in..."));
		return false;
	}

	const IVoiceChatUser* UsersVoiceChatInterface = OnlineSubsystem->GetVoiceChatUserInterface(*LocalUniqueNetID);
	bMuteState = UsersVoiceChatInterface->GetAudioInputDeviceMuted();
	return true;
}
