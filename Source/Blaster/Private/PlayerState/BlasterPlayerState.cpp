// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/BlasterPlayerState.h"
#include "Character/BlasterCharacter.h"
#include "PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, Defeats);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerState::AddToScore(const float ScoreAmount)
{
	if (HasAuthority())
	{
		SetScore(GetScore() + ScoreAmount);
		OnRep_Score();
	}

	/*Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character && Character->Controller)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(Score);
		}
	}*/
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	OnScoreChangedDelegate.Broadcast(GetScore());

	/*Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character && Character->Controller)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(Score);
		}
	}*/
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerState::AddToDefeats(const int32 DefeatsAmount)
{
	SetDefeats(Defeats + DefeatsAmount);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerState::SetDefeats(const int32 NewDefeatsAmount)
{
	if (HasAuthority())
	{
		Defeats = NewDefeatsAmount;
		OnRep_Defeats();
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerState::OnRep_Defeats()
{
	OnDefeatsChangedDelegate.Broadcast(Defeats);
}
