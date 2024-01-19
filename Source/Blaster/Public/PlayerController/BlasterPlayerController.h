// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

class ABlasterHUD;

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	virtual void OnPossess(APawn* InPawn) override;

	void SetHUDHealth(const float Health, const float MaxHealth);

	void SetHUDScore(const float Score);

protected:

	virtual void BeginPlay() override;

private:

	ABlasterHUD* BlasterHUD;
};
