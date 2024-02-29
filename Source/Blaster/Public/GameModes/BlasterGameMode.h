// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

class ABlasterCharacter;
class ABlasterPlayerController;

namespace MatchState
{
	extern BLASTER_API const FName Cooldown; // Match duration has been reached. Display winner and begin cooldown timer
}

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	ABlasterGameMode();

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime;

	float LevelStartingTime;

	virtual void Tick(float DeltaTime) override;

	virtual void OnMatchStateSet() override;

	virtual void PlayerEliminated(ABlasterCharacter* EliminatedPlayer, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);

	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

protected:

	virtual void BeginPlay() override;

private:

	float CountdownTime;
	
};
