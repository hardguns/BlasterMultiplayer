// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


class ABlasterCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UBuffComponent();
	friend class ABlasterCharacter;
	
	void Heal(const float HealAmount, const float HealingTime);
	
	void BuffSpeed(const float BuffBaseSpeed, float BuffCrouchSpeed, const float BuffTime);
	
	void SetInitialSpeeds(const float BaseSpeed, const float CrouchSpeed);
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	
	virtual void BeginPlay() override;
	
	void HealRampUp(const float DeltaTime);
	
private:
	
	UPROPERTY()
	ABlasterCharacter* Character;
	
	/**
	 * Health buff
	 */
	uint8 bIsHealing : 1;
	
	float HealingRate;
	
	float AmountToHeal;
	
	/**
	 * Speed buff
	 */
	
	FTimerHandle SpeedBuffTimerHandle;
	
	float InitialBaseSpeed;
	
	float InitialCrouchSpeed;
	
	void ResetSpeeds();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpeedBuff(const float BaseSpeed, const float CrouchSpeed);
};
