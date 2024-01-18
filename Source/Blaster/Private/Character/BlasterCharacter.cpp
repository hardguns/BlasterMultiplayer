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
#include "Animation/AnimInstance.h"
#include "Blaster/Blaster.h"
#include "PlayerController/BlasterPlayerController.h"
#include "GameModes/BlasterGameMode.h"
#include "TimerManager.h"

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
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	CameraThreshold = 200.f;

	TurnThreshold = 0.5f;
	MaxTimeToCheckLastMovementReplication = 0.25f;

	MaxHealth = 100.f;
	Health = 100.f;

	ElimDelay = 3.f;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHUDHealth();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > MaxTimeToCheckLastMovementReplication)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}

	HideCameraIfCharacterClose();
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
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);
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
void ABlasterCharacter::PlayFireMontage(const bool bAiming)
{
	if (!IsValid(CombatComponent) || !IsValid(CombatComponent->EquippedWeapon))
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInstance) && IsValid(FireWeaponMontage))
	{
		AnimInstance->Montage_Play(FireWeaponMontage);

		const FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::PlayHitReactMontage()
{
	if (!IsValid(CombatComponent) || !IsValid(CombatComponent->EquippedWeapon))
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInstance) && IsValid(HitReactMontage))
	{
		AnimInstance->Montage_Play(HitReactMontage);

		const FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::PlayElimMontage()
{
	if (ElimMontage)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && ElimMontage)
		{
			AnimInstance->Montage_Play(ElimMontage);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
float ABlasterCharacter::CalculateSpeed() const
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::OnRep_Health()
{
	PlayHitReactMontage();
	UpdateHUDHealth();
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::UpdateDissolveMaterial(const float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
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
void ABlasterCharacter::FireButtonPressed()
{
	if (IsValid(CombatComponent))
	{
		CombatComponent->FireButtonPressed(true);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::FireButtonReleased()
{
	if (IsValid(CombatComponent))
	{
		CombatComponent->FireButtonPressed(false);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (IsValid(CombatComponent) && !IsValid(CombatComponent->EquippedWeapon))
	{
		return;
	}

	const float Speed = CalculateSpeed();
	const bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) //stand still, not jumping
	{
		bRotateRootBone = true;
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
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();

	//To know what happens with pitch
	/*if (HasAuthority() && !IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("AO_Pitch: %f"), AO_Pitch);
	}*/
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::SimulatedProxiesTurn()
{
	if (!IsValid(CombatComponent) || !IsValid(CombatComponent->EquippedWeapon))
	{
		return;
	}

	const float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	bRotateRootBone = false;
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	UE_LOG(LogTemp, Warning, TEXT("ProxyYaw: %f"), ProxyYaw);

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//Map pitch from [270, 360) to [-90, 0)
		const FVector2D InRange(270.f, 360.f);
		const FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
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
void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	SetHealth(FMath::Clamp(Health - Damage, 0.f, MaxHealth));

	if (Health == 0.f)
	{
		if (ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>())
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
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
void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimulatedProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::SetHealth(const float NewHealth)
{
	if (HasAuthority() && Health != NewHealth)
	{
		Health = NewHealth;
		OnRep_Health();
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::Elim()
{
	Multicast_Elim();
	GetWorldTimerManager().SetTimer(ElimTimer, this, &ABlasterCharacter::ElimTimerFinished, ElimDelay);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::Multicast_Elim_Implementation()
{
	bElimmed = true;
	PlayElimMontage();

	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);

		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}

	StartDissolve();
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::ElimTimerFinished()
{
	if (ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>())
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (IsValid(FollowCamera) && (FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (IsValid(CombatComponent) && IsValid(CombatComponent->EquippedWeapon) && IsValid(CombatComponent->EquippedWeapon->GetWeaponMesh()))
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (IsValid(CombatComponent) && IsValid(CombatComponent->EquippedWeapon) && IsValid(CombatComponent->EquippedWeapon->GetWeaponMesh()))
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
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
FVector ABlasterCharacter::GetHitTarget() const
{
	if (!IsValid(CombatComponent))
	{
		return FVector();
	}

	return CombatComponent->HitTarget;
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
