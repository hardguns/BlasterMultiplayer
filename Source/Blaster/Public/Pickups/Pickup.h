// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class USphereComponent;
class USoundCue;

UCLASS()
class BLASTER_API APickup : public AActor
{
	GENERATED_BODY()

public:
	
	APickup();

	virtual void Tick(float DeltaTime) override;

protected:
	
	UPROPERTY(VisibleAnywhere)
	UNiagaraComponent* PickupEffectComponent;
	
	UPROPERTY(EditAnywhere)
	UNiagaraSystem* PickupEffect;
	
	UPROPERTY(EditAnywhere)
	float BaseTurnRate;
	
	virtual void BeginPlay() override;
	
	virtual void Destroyed() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:

	UPROPERTY(EditAnywhere)
	USphereComponent* OverlapSphere;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickupMesh;

	UPROPERTY(EditAnywhere)
	USoundCue* PickupSound;

public:
	
};
