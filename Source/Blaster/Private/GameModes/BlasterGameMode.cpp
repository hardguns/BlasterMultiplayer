// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/BlasterGameMode.h"
#include "Character/BlasterCharacter.h"
#include "PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "PlayerState/BlasterPlayerState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

//-----------------------------------------------------------------------------------------------------------------------------------
ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;
	WarmupTime = 10.f;
	MatchTime = 120.f;
	CooldownTime = 10.f;
	CountdownTime = 0.f;
	LevelStartingTime = 0.f;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
		
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* BlasterController = Cast<ABlasterPlayerController>(*It);
		if (BlasterController)
		{
			BlasterController->OnMatchStateSet(MatchState);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* EliminatedPlayer, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	if (EliminatedPlayer)
	{
		EliminatedPlayer->Elim();
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}

	if (ElimmedController)
	{
		TArray<AActor*> OutPlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), OutPlayerStarts);

		int32 Selection = FMath::RandRange(0, OutPlayerStarts.Num() - 1);
		
		if (AActor* RandomPlayerStart = OutPlayerStarts.Num() > 0 ? OutPlayerStarts[Selection] : nullptr)
		{
			RestartPlayerAtPlayerStart(ElimmedController, RandomPlayerStart);
		}
	}
}
