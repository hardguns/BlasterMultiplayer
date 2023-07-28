// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial		UMETA(DisplayName = "Initial State"),
	EWS_Equipped	UMETA(DisplayName = "Equipped"),
	EWS_Dropped		UMETA(DisplayName = "Dropped"),

	EWS_MAX			UMETA(DisplayName = "DefaultMAX"),
};

class USphereComponent;
class UWidgetComponent;

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();

	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void ShowPickupWidget(const bool bShowWidget);

protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:

	UPROPERTY(VisibleAnywhere, Category = "Weapon properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon properties")
	USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, Category = "Weapon properties")
	UWidgetComponent* PickupWidget;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_WeaponState)
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

public:	

	void SetWeaponState(const EWeaponState State);
	
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
};