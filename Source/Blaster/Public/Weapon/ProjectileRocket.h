// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile.h"
#include "ProjectileRocket.generated.h"

class UStaticMeshComponent;

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

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

private:

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* RocketMesh;
	
};
