// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BlasterTypes/TurningInPlace.h"
#include "Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "BlasterTypes/CombatState.h"
#include "BlasterCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlasterOnPlayerEliminatedSignature, const float, RespawnTime);

class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
class AWeapon;
class UCombatComponent;
class UAnimMontage;
class ABlasterPlayerController;
class UParticleSystem;
class USoundCue;
class ABlasterPlayerState;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:

	ABlasterCharacter();

	FBlasterOnPlayerEliminatedSignature OnPlayerEliminatedDelegate;

	UPROPERTY(Replicated)
	bool bDisableGameplay;

	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	void PlayReloadMontage();

	void PlayFireMontage(const bool bAiming);

	void PlayHitReactMontage();

	void PlayElimMontage();

	virtual void OnRep_ReplicatedMovement() override;

	void SetHealth(const float NewHealth);

	// When player gets eliminated
	void Elim();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Elim();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(const bool bShowScope);

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
	void ReloadButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void FireButtonPressed();
	void FireButtonReleased();

#pragma endregion Input actions

	void AimOffset(float DeltaTime);

	void CalculateAO_Pitch();

	void SimulatedProxiesTurn();

	virtual void Jump() override;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);

	void UpdateHUDHealth();

	// Poll for any relevant classes and initialize our HUD
	void PollInit();

	void RotateInPlace(const float DeltaTime);

private:

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatComponent;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	/**
	* Animation montages
	*/

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* ReloadMontage;

	FTimerHandle ElimTimer;

	ABlasterPlayerState* BlasterPlayerState;

	UPROPERTY(EditDefaultsOnly, Category = "Elim")
	float ElimDelay;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server, Reliable)
	void Server_EquipButtonPressed();

	void ElimTimerFinished();

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

	bool bElimmed = false;

	ABlasterPlayerController* BlasterPlayerController;

	UFUNCTION()
	void OnRep_Health();

	/**
	* Dissolve Effect
	*/

	// Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = "Elim")
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	// Material instance set on the blueprint, used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = "Elim")
	UMaterialInstance* DissolveMaterialInstance;

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(const float DissolveValue);

	void StartDissolve();

	/**
	*  Elim Bot
	*/

	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	USoundCue* ElimBotSound;

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

	FORCEINLINE bool IsElimmed() const { return bElimmed; }

	FORCEINLINE float GetHealth() const { return Health; }

	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComponent; }

	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }

	ECombatState GetCombatState() const;
};
