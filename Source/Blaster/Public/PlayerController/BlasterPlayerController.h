// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlasterOnPawnChanged, ABlasterPlayerController*, BlasterPlayerController);

class ABlasterHUD;

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	FBlasterOnPawnChanged OnPawnChangedDelegate;

	virtual void OnPossess(APawn* InPawn) override;

	virtual void SetPawn(APawn* InPawn) override;

	void SetHUDHealth(const float Health, const float MaxHealth);

	void SetHUDScore(const float Score);

	void SetHUDDefeats(const int32 Defeats);

	void SetHUDWeaponAmmo(const int32 WeaponAmmo);

	void SetHUDCarriedAmmo(const int32 CarriedAmmo);

protected:

	virtual void BeginPlay() override;

private:

	ABlasterHUD* BlasterHUD;
};
