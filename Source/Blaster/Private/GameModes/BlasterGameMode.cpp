// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/BlasterGameMode.h"
#include "Character/BlasterCharacter.h"
#include "PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* EliminatedPlayer, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
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
