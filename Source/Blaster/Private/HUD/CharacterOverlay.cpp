// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/CharacterOverlay.h"
#include "PlayerState/BlasterPlayerState.h"
#include "UMG/Public/Components/TextBlock.h"
#include "PlayerController/BlasterPlayerController.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/KismetSystemLibrary.h"

//-----------------------------------------------------------------------------------------------------------------------------------
void UCharacterOverlay::NativeConstruct()
{
	Super::NativeConstruct();

	BindPlayerStateDelegates();
	BindControllerDelegates();
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
void UCharacterOverlay::BindControllerDelegates()
{
	ABlasterPlayerController* BlasterPlayerController = GetOwningPlayer<ABlasterPlayerController>();
	if (BlasterPlayerController)
	{
		BlasterPlayerController->OnPawnChangedDelegate.AddDynamic(this, &ThisClass::OnPlayerPawnChanged);
		OnPlayerPawnChanged(BlasterPlayerController);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCharacterOverlay::OnPawnChanged(APawn* LastPawn, APawn* InPawn)
{
	ABlasterCharacter* PreviousBlasterCharacter = Cast<ABlasterCharacter>(LastPawn);
	if (PreviousBlasterCharacter)
	{
		PreviousBlasterCharacter->OnPlayerEliminatedDelegate.RemoveDynamic(this, &ThisClass::OnPlayerEliminated);
	}


	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if (BlasterCharacter)
	{
		BlasterCharacter->OnPlayerEliminatedDelegate.AddDynamic(this, &ThisClass::OnPlayerEliminated);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCharacterOverlay::OnPlayerPawnChanged(ABlasterPlayerController* BlasterPlayerController)
{
	APawn* CurrentPawn = BlasterPlayerController ? BlasterPlayerController->GetPawn() : nullptr;
	OnPawnChanged(LastPlayerPawn.Get(), CurrentPawn);
	LastPlayerPawn = CurrentPawn;
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

//-----------------------------------------------------------------------------------------------------------------------------------
void UCharacterOverlay::OnPlayerEliminated(const float RespawnTime)
{
	SetElimmedTextsVisibility(ESlateVisibility::Visible);

	CurrentRespawnTime = RespawnTime;
	if (SpawningSecondsText)
	{
		FString TimeText = FString::Printf(TEXT("%d"), FMath::FloorToInt(CurrentRespawnTime));
		SpawningSecondsText->SetText(FText::FromString(TimeText));
	}


	GetWorld()->GetTimerManager().SetTimer(Respawn_TimerHandle, this, &ThisClass::UpdateRespawnTime, 1.f, true);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCharacterOverlay::UpdateRespawnTime()
{
	CurrentRespawnTime -= 1.f;
	if (SpawningSecondsText)
	{
		FString TimeText = FString::Printf(TEXT("%d"), FMath::FloorToInt(CurrentRespawnTime));
		SpawningSecondsText->SetText(FText::FromString(TimeText));
	}

	if (CurrentRespawnTime <= 0.f)
	{
		SetElimmedTextsVisibility(ESlateVisibility::Collapsed);
		GetWorld()->GetTimerManager().ClearTimer(Respawn_TimerHandle);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCharacterOverlay::SetElimmedTextsVisibility(const ESlateVisibility NewVisibility)
{
	if (ElimmedText)
	{
		ElimmedText->SetVisibility(NewVisibility);
	}

	if (SpawningText)
	{
		SpawningText->SetVisibility(NewVisibility);
	}

	if (SpawningSecondsText)
	{
		SpawningSecondsText->SetVisibility(NewVisibility);
	}
}

