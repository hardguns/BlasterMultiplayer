// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UENUM(BlueprintType)
enum class EHitObject : uint8
{
	EHO_None		UMETA(DisplayName = "None"),
	EHO_Character	UMETA(DisplayName = "Character Hit"),
	EHO_Surface		UMETA(DisplayName = "Surface Hit"),

	EHO_MAX			UMETA(DisplayName = "DefaultMAX"),
};

class UBoxComponent;
class UProjectileMovementComponent;
class UParticleSystem;
class UParticleSystemComponent;
class USoundCue;
class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();

protected:

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY()
	UNiagaraComponent* TrailSystemComponent;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* TrailSystem;

	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox;
	
	UPROPERTY(EditAnywhere)
	float Damage;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DamageInnerRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DamageOuterRadius;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* CharacterImpactParticles;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundCue* ImpactSound;

	EHitObject CurrentHitObject;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComponent;

	virtual void BeginPlay() override;

	void DestroyTimerFinished();

	void StartDestroyTimer();

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void SpawnHitParticle();

	void SpawnHitEffects();

	void SpawnTrailSystem();

	void ExplodeDamage();

private:

	UParticleSystemComponent* TracerComponent;

	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;

};
