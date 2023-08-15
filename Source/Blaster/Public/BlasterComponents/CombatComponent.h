// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/BlasterHUD.h"
#include "CombatComponent.generated.h"

//#define TRACE_LENGTH 80000.f;

class AWeapon;
class ABlasterCharacter;
class ABlasterPlayerController;
class ABlasterHUD;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	friend class ABlasterCharacter;

	void EquipWeapon(AWeapon* WeaponToEquip);

protected:
	virtual void BeginPlay() override;

	void SetAiming(const bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void Server_SetAiming(const bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void FireButtonPressed(const bool bPressed);

	void Fire();

	UFUNCTION(Server, Reliable)
	void Server_Fire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Fire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(const float DeltaTime);

private:

	UPROPERTY(EditDefaultsOnly, Category = "Equip")
	FName HandSocketName;

	ABlasterCharacter* Character;

	ABlasterPlayerController* Controller;

	ABlasterHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	FVector HitTarget;

	UPROPERTY(Replicated)
	bool bAiming;

	bool bFireButtonPressed;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;
	
	UPROPERTY(EditAnywhere, Category = "Trace")
	float TraceDistance;

	/** Offset that will be added to the start location of the line trace that checks for enemies */
	UPROPERTY(EditAnywhere, Category = "Trace")
	float TraceLocationForwardOffset;

	/**
	* HUD and crosshairs
	*/

	FHUDPackage HUDPackage;

	/** Amount that sets crosshair spread based on velocity */
	float CrosshairVelocityFactor;

	/** Amount that sets crosshair spread based on air state */
	float CrosshairInAirFactor;

	/** Amount that sets crosshair spread based on aiming state */
	float CrosshairAimFactor;

	/** Amount that sets crosshair spread based on shooting state */
	float CrosshairShootingFactor;

	UPROPERTY(EditAnywhere, Category = "Crosshair")
	float BaseLineSpread;

	UPROPERTY(EditAnywhere, Category = "Crosshair")
	float CrosshairAimAffectValue;

	UPROPERTY(EditAnywhere, Category = "Crosshair")
	float CrosshairShootAffectValue;

	/**
	* Aiming and FOV
	*/

	//Field of view when not aiming; set to the camera's base FOV in BeginPlay
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	/**
	*  Automatic Fire
	*/

	FTimerHandle FireTimer;

	bool bCanFire;

	void StartFireTimer();

	void FireTimerFinished();

public:	
	

};
