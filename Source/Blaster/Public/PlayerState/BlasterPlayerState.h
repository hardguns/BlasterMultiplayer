// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

class ABlasterCharacter;
class ABlasterPlayerController;

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()

public: 

	virtual void OnRep_Score() override;

	void AddToScore(const float ScoreAmount);

	void UpdateHUDScore();

private:

	ABlasterCharacter* Character;

	ABlasterPlayerController* Controller;
	
};
