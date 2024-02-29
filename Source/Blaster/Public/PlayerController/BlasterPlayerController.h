// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlasterOnPawnChanged, ABlasterPlayerController*, BlasterPlayerController);

class ABlasterHUD;
class UCharacterOverlay;

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	ABlasterPlayerController();

	FBlasterOnPawnChanged OnPawnChangedDelegate;

	virtual void OnPossess(APawn* InPawn) override;

	virtual void SetPawn(APawn* InPawn) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetHUDHealth(const float Health, const float MaxHealth);

	void SetHUDScore(const float Score);

	void SetHUDDefeats(const int32 Defeats);

	void SetHUDWeaponAmmo(const int32 WeaponAmmo);

	void SetHUDCarriedAmmo(const int32 CarriedAmmo);

	void SetHUDMatchCountdown(const float CountdownTime);

	void SetHUDAnnouncementCountdown(const float CountdownTime);

	virtual float GetServerTime(); // Sync with server world clock

	virtual void ReceivedPlayer() override; // Sync with server clock as soon as possible

	float GetTimeLeftByState();

	void OnMatchStateSet(const FName& State);

	void HandleMatchHasStarted();

	void HandleCooldown();

protected:

	float ClientServerDelta;

	UPROPERTY(EditAnywhere, Category = "Time")
	float TimeSyncFrequency;

	float TimeSyncRunningTime;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	void SetHUDTime();

	void PollInit();

	/** 
	* Sync time between client and server
	*/

	// Requests the current server time, passing in the client's time when request was sent
	UFUNCTION(Server, Reliable)
	void Server_RequestServerTime(const float TimeOfClientRequest);

	// Reports the current server time to the client in response to Server_RequestServerTime
	UFUNCTION(Client, Reliable)
	void Client_ReportServerTime(const float TimeOfClientRequest, const float TimeServerReceivedClientRequest);

	void CheckTimeSync(const float DeltaTime);

	UFUNCTION(Server, Reliable)
	void Server_CheckMatchState();

	UFUNCTION(Client, Reliable)
	void Client_JoinMidgame(const FName& StateOfMatch, const float Warmup, const float Match, const float StartingTime);

private:

	ABlasterHUD* BlasterHUD;

	float MatchTime;

	float WarmupTime;

	float LevelStartingTime;

	uint32 CountdownInt;

	bool bInitializeCharacterOverlay;

	/** TODO: CHANGE THIS TO OVERLAY DELEGATE */
	float HUDHealth;

	float HUDMaxHealth;

	/*******************************************/

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

};
