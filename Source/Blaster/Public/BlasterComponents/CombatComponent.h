// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/BlasterHUD.h"
#include "Weapon/WeaponTypes.h"
#include "BlasterTypes/CombatState.h"
#include "CombatComponent.generated.h"

class AProjectile;
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

	void Reload();

	void FireButtonPressed(const bool bPressed);

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	void JumpToShotgunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_LaunchGrenade(const FVector_NetQuantize& Target);

protected:

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AProjectile> GrenadeClass;
	
	virtual void BeginPlay() override;

	void SetAiming(const bool bIsAiming);

	void SetEquippedWeapon(AWeapon* WeaponToEquip);

	UFUNCTION(Server, Reliable)
	void Server_SetAiming(const bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void Fire();

	UFUNCTION(Server, Reliable)
	void Server_Fire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Fire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Reload();

	void HandleReload();

	int32 AmountToReload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(const float DeltaTime);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ThrowGrenade();

	void ThrowGrenade();

	void DropEquippedWeapon();

	void AttachActorToSocket(AActor* ActorToAttach, const FName& SocketToAttachTo);

	void AttachActorToLeftHand(AActor* ActorToAttach);

	void UpdateCarriedAmmo();

	void PlayEquippedWeaponSound();

	void ReloadEmptyWeapon();
	
	void ShowAttachedGrenade(const bool bShowGrenade);

private:

	UPROPERTY(EditDefaultsOnly, Category = "Equip")
	FName LeftHandSmallWeaponSocketName;
	
	UPROPERTY(EditDefaultsOnly, Category = "Equip")
	FName LeftHandSocketName;

	UPROPERTY(EditDefaultsOnly, Category = "Equip")
	FName RightHandSocketName;

	ABlasterCharacter* Character;

	ABlasterPlayerController* Controller;

	ABlasterHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

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

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo;

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo;

	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo;

	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo;

	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo;

	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo;

	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo;

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState;

	void SetCarriedAmmo(const EWeaponType CurrentWeaponType);

	UFUNCTION()
	void OnRep_CarriedAmmo();

	void InitializeCarriedAmmo();

	void SetCombatState(const ECombatState NewCombatState);

	UFUNCTION()
	void OnRep_CombatState();

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

	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades;

	UPROPERTY(EditAnywhere)
	int32 MaxGrenades;

	void SetGrenades(const float NewValue);

	void InterpFOV(float DeltaTime);

	UFUNCTION()
	void OnRep_Grenades();

	/**
	*  Automatic Fire
	*/

	FTimerHandle FireTimer;

	bool bCanFire;

	void StartFireTimer();

	void FireTimerFinished();

	bool CanFire();

	void UpdateAmmoValues();

	void UpdateShotgunAmmoValues();

	void UpdateHUDGrenades();

public:	
	
	int32 GetCarriedAmmo() const { return CarriedAmmo; }

	ECombatState GetCombatState() const { return CombatState; }

	FORCEINLINE int32 GetGrenades() const { return Grenades; }

};
