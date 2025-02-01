// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "IOnlineSubsystemEOS.h"

#include "EOSSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEOS, Log, All);



DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEOSOnMasterClientChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEOSOnAppEntersForeground);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOSOnLogin, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOSOnLogEntry, FString, Log);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOSOnRoomDestroyed, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOSOnPlayerJoined, FString, playerNetId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEOSOnPlayerLeft, FString, playerNetId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOSOnFindRoom, bool, bWasSuccessful, FString, ErrMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOSOnCreateRoom, bool, bWasSuccessful, bool, isMasterClient);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOSOnJoinRoom, bool, bWasSuccessful, FString, hostAddress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOSOnSessionStarted, FString, SessionName, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEOSOnEndSession, FString, SessionName, bool, bWasSuccessful);



// Copied from: OnlineSubsystemTypes.h and expose to blueprints.
UENUM(BlueprintType)
enum class ESessionState : uint8
{
	/** An online session has not been created yet */
	NoSession,
	/** An online session is in the process of being created */
	Creating,
	/** Session has been created but the session hasn't started (pre match lobby) */
	Pending,
	/** Session has been asked to start (may take time due to communication with backend) */
	Starting,
	/** The current session has started. Sessions with join in progress disabled are no longer joinable */
	InProgress,
	/** The session is still valid, but the session is no longer being played (post match lobby) */
	Ending,
	/** The session is closed and any stats committed */
	Ended,
	/** The session is being destroyed */
	Destroying,
	/** This indicates that the Session State value is Invalid and should not be relied on.
	IF game code sees this value, it needs to be investigated... Likely bad conversion from internal state value.
	*/
	Invalid
};



UCLASS()
class UNREALDISCOVER_API UEOSSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category = "EOS|General")
	FEOSOnLogin OnLogin;

	UPROPERTY(BlueprintAssignable, Category = "EOS|Room")
	FEOSOnCreateRoom OnRoomCreated;

	/// <summary>
	/// bWasSuccessful: wether a room was found or not
	/// ErrMessage: IF bWasSuccessful = false THEN this is the error message.
	/// </summary>
	UPROPERTY(BlueprintAssignable, Category = "EOS|Room")
	FEOSOnFindRoom OnRoomFound;

	UPROPERTY(BlueprintAssignable, Category = "EOS|Room")
	FEOSOnJoinRoom OnRoomJoined;

	UPROPERTY(BlueprintAssignable, Category = "EOS|Log")
	FEOSOnLogEntry OnLogEntry;

	UPROPERTY(BlueprintAssignable, Category = "EOS|Room")
	FEOSOnMasterClientChanged OnMasterClientChanged;

	UPROPERTY(BlueprintAssignable, Category = "EOS|Room")
	FEOSOnRoomDestroyed OnRoomDestroyed;

	UPROPERTY(BlueprintAssignable, Category = "EOS|General")
	FEOSOnPlayerJoined OnPlayerJoined;

	UPROPERTY(BlueprintAssignable, Category = "EOS|General")
	FEOSOnPlayerLeft OnPlayerLeft;

	UPROPERTY(BlueprintAssignable, Category = "EOS|General")
	FEOSOnSessionStarted OnSessionStarted;

	UPROPERTY(BlueprintAssignable, Category = "EOS|General")
	FEOSOnSessionStarted FEOSOnEndSession;

	UPROPERTY(BlueprintAssignable, Category = "EOS|General")
	FEOSOnAppEntersForeground OnAppEntersForeground;

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "EOS|General")
	void Login();

	UFUNCTION(BlueprintCallable, Category = "EOS|Session")
	void CreateSession(const FName& SessionName, const bool bWithVoiceChat);

	/// <summary>
	/// Call this to move the session state from Pending -> InProgress
	/// </summary>
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "EOS|Session")
	void StartSession();

	/// <summary>
	/// Call to move the session state from InProgress -> Ended.
	/// Once this is called, you must destroy the session in order to create a new session.
	/// </summary>
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "EOS|Session")
	void EndSession();

	//If session is found the session name is saved.
	UFUNCTION(BlueprintCallable, Category = "EOS|Session")
	void FindSession(const FName& SessionName);

	UFUNCTION(BlueprintCallable, Category = "EOS|Session")
	void JoinSession();

	UFUNCTION(BlueprintCallable, Category = "EOS|Session")
	void UpdateJoinability(const bool Joinable);

	//If the user is the only one in the session, the session is destroyed. If they user is not the only one if the session, the user leaves.
	UFUNCTION(BlueprintCallable, Category = "EOS|Session")
	void DestroySession();

	UFUNCTION(BlueprintCallable, Category = "EOS|General")
	FString GetNetID();

	UFUNCTION(BlueprintCallable, Category = "EOS|General")
	FString GetHostAddress();

	void NotifyGameInstanceThatEOSInitialized(bool bEOSInit) const;

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "EOS|General")
	void OnHostSuccess(UObject* WorldContextObject, FString LevelName);

	UFUNCTION(BlueprintPure, Category = "EOS|General")
	FName GetCurrentSessionName() const { return CurrentSessionName; }

	UFUNCTION(BlueprintCallable, Category = "EOS|Session")
	ESessionState GetSessionState() const;


	UFUNCTION(BlueprintCallable, Category = "EOS|VoiceChat")
	const bool SetLocalPlayerVoiceChatMuteState(const bool bNewMuted);

	UFUNCTION(BlueprintCallable, Category = "EOS|VoiceChat")
	const bool GetLocalPlayerVoiceChatMuteState(bool& bMuteState) const;

	UFUNCTION(BlueprintCallable, Category = "EOS|VoiceChat")
	const bool IsInVoiceChat() const;

private:
	IOnlineSubsystemEOS* OnlineSubsystem;
	IOnlineIdentityPtr IdentityPtr;
	IOnlineSessionPtr SessionPtr;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
	FOnlineSessionSearchResult SearchResult;
	FName CurrentSessionName;
	
	bool bIsMasterClient;
	bool bEnterForegroundFirstTime;
	int32 BackgroundCounter;

private:
	const FUniqueNetId* GetLocalUniqueNetID(bool bOnlyIfLoggedIn) const;
	void OnLoginComplete(int32 NumPlayers, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	void OnCreateSessionComplete(FName NewSessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName JoinedSessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnPlayerJoinedSession(FName SessionName, const FUniqueNetId& PlayerJoinedId);
	void OnPlayerLeftSession(FName SessionName, const FUniqueNetId& PlayerLeftId, EOnSessionParticipantLeftReason Reason);
	void OnPlayerJoinOrLeftSession(FName SessionName, const FUniqueNetId& UserId, bool bJoined);
	void OnSessionStartedInternal(FName SessionName, bool bWasSuccessful);
	void OnSessionEndedInternal(FName SessionName, bool bWasSuccessful);
	void OnSessionSettingsUpdatedInternal(FName SessionName, const FOnlineSessionSettings& SessionSettings);
	void OnUpdateSessionCompleteInternal(FName SessionName, bool WasSuccessful);
	void LogEntry(const FString& LogEntry, bool bBroadcast = true);
	void LogWarning(const FString& LogEntry, bool bBroadcast = true);
	void LogError(const FString& LogEntry, bool bBroadcast = true);
	void HandleApplicationEnterBackground();
	void HandleApplicationEnterForeground();
};
