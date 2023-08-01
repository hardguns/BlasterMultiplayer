// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"
#include "BlasterComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"

//-----------------------------------------------------------------------------------------------------------------------------------
ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (IsValid(CombatComponent))
	{
		CombatComponent->Character = this;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::EquipButtonPressed()
{
	if (IsValid(CombatComponent))
	{
		if (HasAuthority())
		{
			CombatComponent->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			Server_EquipButtonPressed();
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::Server_EquipButtonPressed_Implementation()
{
	if (IsValid(CombatComponent))
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
		return;
	}

	Crouch();
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::AimButtonPressed()
{
	if (IsValid(CombatComponent))
	{
		CombatComponent->SetAiming(true);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::AimButtonReleased()
{
	if (IsValid(CombatComponent))
	{
		CombatComponent->SetAiming(false);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (IsValid(CombatComponent) && !IsValid(CombatComponent->EquippedWeapon))
	{
		return;
	}

	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	const float Speed = Velocity.Size();

	const bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) //stand still, not jumping
	{
		const FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		const FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}

		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}

	if (Speed > 0.f || bIsInAir) // running or jumping
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//Map pitch from [270, 360) to [-90, 0)
		const FVector2D InRange(270.f, 360.f);
		const FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}

	//To know what happens with pitch
	/*if (HasAuthority() && !IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("AO_Pitch: %f"), AO_Pitch);
	}*/
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::TurnInPlace(const float DeltaTime)
{
	//UE_LOG(LogTemp, Warning, TEXT("AO_Yaw: %f"), AO_Yaw);
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;

		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (IsValid(OverlappingWeapon))
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (IsValid(OverlappingWeapon))
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
bool ABlasterCharacter::IsWeaponEquipped() const
{
	return (IsValid(CombatComponent) && CombatComponent->EquippedWeapon);
}

//-----------------------------------------------------------------------------------------------------------------------------------
bool ABlasterCharacter::IsAiming() const
{
	return (IsValid(CombatComponent) && CombatComponent->bAiming);
}

//-----------------------------------------------------------------------------------------------------------------------------------
AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if (!IsValid(CombatComponent))
	{
		return nullptr;
	}

	return CombatComponent->EquippedWeapon;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (IsValid(OverlappingWeapon))
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	
	if (IsValid(LastWeapon))
	{
		LastWeapon->ShowPickupWidget(false);
	}
}
