// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/BlasterPlayerState.h"
#include "Character/BlasterCharacter.h"
#include "PlayerController/BlasterPlayerController.h"

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	UpdateHUDScore();
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerState::AddToScore(const float ScoreAmount)
{
	if (HasAuthority())
	{
		SetScore(GetScore() + ScoreAmount);
		OnRep_Score();
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterPlayerState::UpdateHUDScore()
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character && Character->Controller)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}
