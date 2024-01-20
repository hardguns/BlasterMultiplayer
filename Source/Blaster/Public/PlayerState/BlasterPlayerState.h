// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

class ABlasterCharacter;
class ABlasterPlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlasterOnScoreChangedSignature, const float, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlasterOnDefeatsChangedSignature, const int32, NewDefeats);

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY() 

public: 

	FBlasterOnScoreChangedSignature OnScoreChangedDelegate;

	FBlasterOnDefeatsChangedSignature OnDefeatsChangedDelegate;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	/**
	* Replication notifies
	*/
	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_Defeats();

	void AddToScore(const float ScoreAmount);

	void AddToDefeats(const int32 DefeatsAmount);

	void SetDefeats(const int32 NewDefeatsAmount);

	int32 GetDefeats() const { return Defeats; }

private:

	UPROPERTY(Transient)
	ABlasterCharacter* Character;

	UPROPERTY(Transient)
	ABlasterPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;
};
