// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

class UProgressBar;
class UTextBlock;
class ABlasterPlayerController;

/**
 * 
 */
UCLASS()
class BLASTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoAmount;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ElimmedText;
	
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* SpawningText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* SpawningSecondsText;

protected:

	UPROPERTY(Transient)
	TObjectPtr<APawn> LastPlayerPawn;

	float CurrentRespawnTime;

	FTimerHandle Respawn_TimerHandle;

	void BindPlayerStateDelegates();

	void BindControllerDelegates();

	void OnPawnChanged(APawn* LastPawn, APawn* InPawn);

	UFUNCTION()
	void OnPlayerPawnChanged(ABlasterPlayerController* BlasterPlayerController);

	UFUNCTION()
	void UpdateScore(const float NewScore);

	UFUNCTION()
	void UpdateDefeats(const int32 NewDefeats);
	
	UFUNCTION()
	void OnPlayerEliminated(const float RespawnTime);

	UFUNCTION()
	void UpdateRespawnTime();

	void SetElimmedTextsVisibility(const ESlateVisibility NewVisibility);
	
};
