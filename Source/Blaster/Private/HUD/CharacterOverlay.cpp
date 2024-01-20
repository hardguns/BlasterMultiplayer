// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/CharacterOverlay.h"
#include "PlayerState/BlasterPlayerState.h"
#include "UMG/Public/Components/TextBlock.h"

//-----------------------------------------------------------------------------------------------------------------------------------
void UCharacterOverlay::NativeConstruct()
{
	Super::NativeConstruct();

	BindPlayerStateDelegates();
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCharacterOverlay::BindPlayerStateDelegates()
{
	ABlasterPlayerState* PlayerState = GetOwningPlayerState<ABlasterPlayerState>();
	if (!PlayerState)
	{
		FTimerHandle SearchPlayerState_TimerHandle;
		FTimerDelegate SearchPlayerStateDelegate = FTimerDelegate::CreateUObject(this, &UCharacterOverlay::BindPlayerStateDelegates);
		GetWorld()->GetTimerManager().SetTimer(SearchPlayerState_TimerHandle, SearchPlayerStateDelegate, 0.5f, false);
		return;
	}

	PlayerState->OnScoreChangedDelegate.AddDynamic(this, &UCharacterOverlay::UpdateScore);
	PlayerState->OnDefeatsChangedDelegate.AddDynamic(this, &UCharacterOverlay::UpdateDefeats);

	// Setting score and defeats for the first time
	UpdateScore(PlayerState->GetScore());
	UpdateDefeats(PlayerState->GetDefeats());
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCharacterOverlay::UpdateScore(const float NewScore)
{
	if (ScoreAmount)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(NewScore));
		ScoreAmount->SetText(FText::FromString(ScoreText));
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCharacterOverlay::UpdateDefeats(const int32 NewDefeats)
{
	if (DefeatsAmount)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), NewDefeats);
		DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
}

