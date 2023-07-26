// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

//-----------------------------------------------------------------------------------------------------------------------------------
void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	if (IsValid(InPawn))
	{
		ENetRole LocalRole = InPawn->GetLocalRole();
		FString Role;

		switch (LocalRole)
		{
		case ENetRole::ROLE_Authority:
			Role = FString("Authority");
			break;
		case ENetRole::ROLE_AutonomousProxy:
			Role = FString("Autonomous Proxy");
			break;
		case ENetRole::ROLE_SimulatedProxy:
			Role = FString("Simulated Proxy");
			break;
		case ENetRole::ROLE_None:
			Role = FString("None");
			break;
		}

		FString LocalRoleString = FString::Printf(TEXT("Local Role: %s"), *Role);
		SetDisplayText(LocalRoleString);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UOverheadWidget::ShowPlayerName(APawn* InPawn)
{
	if (!IsValid(InPawn))
	{
		SetDisplayText(FString("Not found!"));
		return;
	}

	const APlayerState* PlayerState = InPawn->GetPlayerState();
	if (!IsValid(PlayerState) || !*PlayerState->GetPlayerName())
	{
		FTimerHandle SearchPlayerState_TimerHandle;
		FTimerDelegate SearchPlayerStateDelegate = FTimerDelegate::CreateUObject(this, &UOverheadWidget::ShowPlayerName, InPawn);
		GetWorld()->GetTimerManager().SetTimer(SearchPlayerState_TimerHandle, SearchPlayerStateDelegate, 0.5f, false);
		return;
	}
		
	const FString PlayerName = PlayerState->GetPlayerName();
	SetDisplayText(PlayerName);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();

	Super::NativeDestruct();
}
