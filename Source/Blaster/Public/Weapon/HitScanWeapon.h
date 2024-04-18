// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

class UParticleSystem;
class USoundCue;

UCLASS()
class BLASTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	
	AHitScanWeapon();

	virtual void Fire(const FVector& HitTarget) override;
	
protected:

	UPROPERTY(EditAnywhere)
	float Damage;
	
	UPROPERTY(EditAnywhere, Category = "Fire")
	FName FlashSocketName;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	/**
	* In case, weapon does not have an animation associated particles and SFX will be spawned from class 
	 */
	UPROPERTY(EditAnywhere)
	USoundCue* HitSound;

	//--------------------------------------------------------------------------------------------------
	
	virtual void BeginPlay() override;

	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);

	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

private:

	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

	/**
	 * In case, weapon does not have an animation associated particles and SFX will be spawned from class 
	 */
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;

	/**
	 * Trace end with scatter
	 */

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter;
};
