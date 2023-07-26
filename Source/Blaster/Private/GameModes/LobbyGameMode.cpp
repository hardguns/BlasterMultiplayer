// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/KismetSystemLibrary.h"

//-----------------------------------------------------------------------------------------------------------------------------------
void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UKismetSystemLibrary::PrintString(GetWorld(), FString::SanitizeFloat(GameState.Get()->PlayerArray.Num()));

	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	if (NumberOfPlayers == 2)
	{
		UWorld* World = GetWorld();
		if (IsValid(World))
		{
			bUseSeamlessTravel = true;
			World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
		}
	}
}
