// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/BlasterPlayerController.h"
#include "HUD/BlasterHUD.h"
#include "HUD/CharacterOverlay.h"
#include "HUD/Announcement.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "BlasterComponents/CombatComponent.h"
#include "GameState/BlasterGameState.h"
#include "PlayerState/BlasterPlayerState.h"
#include "Character/BlasterCharacter.h"
#include "GameModes/BlasterGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"


//-----------------------------------------------------------------------------------------------------------------------------------
ABlasterPlayerController::ABlasterPlayerController()
{
	MatchTime = 0.f;
	WarmupTime = 0.f;
	CooldownTime = 0.f;
	CountdownInt = 0;

	ClientServerDelta = 0.f;
	TimeSyncFrequency = 5.f;
	TimeSyncRunningTime = 0.f;

	bInitializeCharacterOverlay = false;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	Server_CheckMatchState();
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (const ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn))
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());

		if (BlasterCharacter->GetCombatComponent())
		{
			SetHUDGrenades(BlasterCharacter->GetCombatComponent()->GetGrenades());
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	OnPawnChangedDelegate.Broadcast(this);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::SetHUDHealth(const float Health, const float MaxHealth)
{
	BlasterHUD = BlasterHUD ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HealthBar &&
		BlasterHUD->CharacterOverlay->HealthText;

	if (bHUDValid)
	{
		const float HealthPercent = MaxHealth > 0 ? Health / MaxHealth : 0.f;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);

		const FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::SetHUDScore(const float Score)
{
	BlasterHUD = BlasterHUD ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ScoreAmount;

	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::SetHUDDefeats(const int32 Defeats)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::SetHUDWeaponAmmo(const int32 WeaponAmmo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount;

	if (bHUDValid)
	{
		FString WeaponAmmoText = FString::Printf(TEXT("%d"), WeaponAmmo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(WeaponAmmoText));
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::SetHUDCarriedAmmo(const int32 CarriedAmmo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount;

	if (bHUDValid)
	{
		const FString CarriedAmmoText = FString::Printf(TEXT("%d"), CarriedAmmo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText));
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::SetHUDWeaponIcon(UTexture2D* NewIcon)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponIcon;

	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->WeaponIcon->SetBrushFromTexture(NewIcon, true);
		BlasterHUD->CharacterOverlay->WeaponIcon->SetColorAndOpacity(NewIcon ? FLinearColor(1.f, 1.f, 1.f, 1.f) : FLinearColor(1.f, 1.f, 1.f, 0.f));
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::SetHUDMatchCountdown(const float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->MatchCountdownText;

	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::SetHUDAnnouncementCountdown(const float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->AnnouncementWidget &&
		BlasterHUD->AnnouncementWidget->WarmupTime;

	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			BlasterHUD->AnnouncementWidget->WarmupTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->AnnouncementWidget->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::SetHUDGrenades(const int32 Grenades)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	const bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->GrenadesText;

	if (bHUDValid)
	{
		const FString CarriedGrenadesText = FString::Printf(TEXT("%d"), Grenades);
		BlasterHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(CarriedGrenadesText));
	}
	else
	{
		HUDGrenades = Grenades;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
float ABlasterPlayerController::GetServerTime()
{
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		Server_RequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
float ABlasterPlayerController::GetTimeLeftByState()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		LevelStartingTime = GameMode->LevelStartingTime;
	}

	if (MatchState == MatchState::WaitingToStart)
	{
		return WarmupTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::InProgress)
	{
		return WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::Cooldown)
	{
		return CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}

	return WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::OnMatchStateSet(const FName& State)
{
	if (HasAuthority() && MatchState != State)
	{
		MatchState = State;
		OnRep_MatchState();
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::HandleMatchHasStarted()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		if (BlasterHUD->CharacterOverlay == nullptr)
		{
			BlasterHUD->AddCharacterOverlay();
		}
		
		if (BlasterHUD->AnnouncementWidget)
		{
			BlasterHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::HandleCooldown()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD && BlasterHUD->CharacterOverlay)
	{
		BlasterHUD->CharacterOverlay->RemoveFromParent();
		const bool bHUDValid = BlasterHUD->AnnouncementWidget 
			&& BlasterHUD->AnnouncementWidget->AnnouncementText 
			&& BlasterHUD->AnnouncementWidget->InfoText;

		if (bHUDValid)
		{
			BlasterHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Visible);

			FString AnnouncementText("New match starts in:");
			BlasterHUD->AnnouncementWidget->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();

			if (BlasterGameState)
			{
				FString InfoTextString;
				TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;
				if (TopPlayers.Num() == 0)
				{
					InfoTextString = FString("There is no winner.");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == BlasterPlayerState)
				{
					InfoTextString = FString("You are the winner!");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0])
				{
					InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *TopPlayers[0]->GetPlayerName());
				}
				else if (TopPlayers.Num() > 1)
				{
					InfoTextString = FString("Players tied for the win: \n");
					for (ABlasterPlayerState* TiedPlayer : TopPlayers)
					{
						if (TiedPlayer)
						{
							InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
						}
					}
				}

				BlasterHUD->AnnouncementWidget->InfoText->SetText(FText::FromString(InfoTextString));
			}

			
		}
	}

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
	if (BlasterCharacter && BlasterCharacter->GetCombatComponent())
	{
		BlasterCharacter->bDisableGameplay = true;
		BlasterCharacter->GetCombatComponent()->FireButtonPressed(false);
		
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft = GetTimeLeftByState();
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}

		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}

	CountdownInt = SecondsLeft;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::PollInit()
{
	if (!BlasterHUD)
	{
		return;
	}
	
	if (CharacterOverlay == nullptr)
	{
		if (!BlasterHUD->CharacterOverlay && MatchState == MatchState::InProgress)
		{
			BlasterHUD->AddCharacterOverlay();
		}
		
		CharacterOverlay = BlasterHUD->CharacterOverlay;
		if (CharacterOverlay)
		{
			SetHUDHealth(HUDHealth, HUDMaxHealth);

			const ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
			if (BlasterCharacter && BlasterCharacter->GetCombatComponent())
			{
				SetHUDGrenades(BlasterCharacter->GetCombatComponent()->GetGrenades());
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::Server_RequestServerTime_Implementation(const float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	Client_ReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::Client_ReportServerTime_Implementation(const float TimeOfClientRequest, const float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);

	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::CheckTimeSync(const float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		Server_RequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::Server_CheckMatchState_Implementation()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		CooldownTime = GameMode->CooldownTime;

		OnMatchStateSet(GameMode->GetMatchState());
		Client_JoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerController::Client_JoinMidgame_Implementation(const FName& StateOfMatch, const float Warmup, const float Match, const float Cooldown, const float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;

	if (MatchState == MatchState::WaitingToStart)
	{
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
		if (BlasterHUD)
		{
			BlasterHUD->AddAnnouncementWidget();
		}
	}
}
