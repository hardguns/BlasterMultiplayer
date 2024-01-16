// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BlasterTypes/TurningInPlace.h"
#include "Interfaces/InteractWithCrosshairsInterface.h"
#include "BlasterCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
class AWeapon;
class UCombatComponent;
class UAnimMontage;
class ABlasterPlayerController;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();

	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	void PlayFireMontage(const bool bAiming);

	void PlayHitReactMontage();
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_Hit();

	virtual void OnRep_ReplicatedMovement() override;

protected:
	virtual void BeginPlay() override;

#pragma region Input actions

	// Input actions
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void FireButtonPressed();
	void FireButtonReleased();

#pragma endregion Input actions

	void AimOffset(float DeltaTime);

	void CalculateAO_Pitch();

	void SimulatedProxiesTurn();

	virtual void Jump() override;

private:

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;

	UPROPERTY(VisibleAnywhere, Category = "Combat")
	UCombatComponent* CombatComponent;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* HitReactMontage;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server, Reliable)
	void Server_EquipButtonPressed();


#pragma region Aim offset

	float AO_Yaw;

	float InterpAO_Yaw;

	float AO_Pitch;

	FRotator StartingAimRotation;

#pragma endregion Aim offset

#pragma region Turn In Place

	ETurningInPlace TurningInPlace;

	bool bRotateRootBone;

	float TurnThreshold;

	float ProxyYaw;

	UPROPERTY(EditDefaultsOnly, Category = "Turn In place|Simulated Proxy")
	float MaxTimeToCheckLastMovementReplication;

	float TimeSinceLastMovementReplication;

	FRotator ProxyRotationLastFrame;

	FRotator ProxyRotation;

	void TurnInPlace(const float DeltaTime);

#pragma endregion Turn In Place

#pragma region Camera

	UPROPERTY(EditAnywhere, Category = "Camera")
	float CameraThreshold;

	void HideCameraIfCharacterClose();

#pragma endregion Camera

	float CalculateSpeed() const;

	/**
	* Player health 
	*/

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health;

	ABlasterPlayerController* BlasterPlayerController;

	UFUNCTION()
	void OnRep_Health();

public:

	void SetOverlappingWeapon(AWeapon* Weapon);

	bool IsWeaponEquipped() const;

	bool IsAiming() const;

	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }

	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }

	AWeapon* GetEquippedWeapon();

	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }

	FVector GetHitTarget() const;

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }

};
