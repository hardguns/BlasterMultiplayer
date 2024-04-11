// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile.h"
#include "ProjectileRocket.generated.h"

class URocketMovementComponent;
class UNiagaraComponent;
class UStaticMeshComponent;
class UNiagaraSystem;
class USoundCue;

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:

	AProjectileRocket();

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DamageInnerRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DamageOuterRadius;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* TrailSystem;

	UPROPERTY()
	UNiagaraComponent* TrailSystemComponent;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundCue* ProjectileLoop;

	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundAttenuation* LoopingSoundAttenuation;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	URocketMovementComponent* RocketMovementComponent;
	
	virtual void BeginPlay() override;

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	virtual void Destroyed() override;

	void DestroyTimerFinished();

private:

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* RocketMesh;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime;
	
};
