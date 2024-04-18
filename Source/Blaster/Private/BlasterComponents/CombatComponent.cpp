// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/CombatComponent.h"
#include "Weapon/Weapon.h"
#include "Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "PlayerController/BlasterPlayerController.h"
#include "HUD/BlasterHUD.h"
#include "Camera/CameraComponent.h"
#include "Sound/SoundCue.h"

//-----------------------------------------------------------------------------------------------------------------------------------
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	HandSocketName = "RightHandSocket";

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;

	TraceDistance = 80000.f;
	TraceLocationForwardOffset = 100.f;

	BaseLineSpread = 0.5f;
	CrosshairAimAffectValue = 0.58f;
	CrosshairShootAffectValue = 0.60f;

	bCanFire = true;
	StartingARAmmo = 30;
	StartingRocketAmmo = 0;
	StartingPistolAmmo = 15;
	StartingSMGAmmo = 20;
	StartingShotgunAmmo = 0;
	StartingSniperAmmo = 0;
	CombatState = ECombatState::ECS_Unoccupied;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		if (IsValid(Character->GetFollowCamera()))
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}

		if (Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}

		Controller = !IsValid(Controller) ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller && Character->IsLocallyControlled())
		{
			Controller->SetHUDWeaponIcon(EquippedWeapon ? EquippedWeapon->WeaponIcon : nullptr);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsValid(Character) && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::SetHUDCrosshairs(const float DeltaTime)
{
	if (!IsValid(Character) || !IsValid(Character->Controller))
	{
		return;
	}

	Controller = !IsValid(Controller) ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (IsValid(Controller))
	{
		HUD = !IsValid(HUD) ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		if (IsValid(HUD))
		{
			if (IsValid(EquippedWeapon))
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
			}

			//Calculate crosshair spread
			//[0, 600] -> [0, 1]
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, CrosshairAimAffectValue, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 35.f);

			HUDPackage.CrosshairSpread = BaseLineSpread + 
				CrosshairVelocityFactor + 
				CrosshairInAirFactor - 
				CrosshairAimFactor + 
				CrosshairShootingFactor;

			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::SetCarriedAmmo(const EWeaponType CurrentWeaponType)
{
	if (Character && Character->HasAuthority())
	{
		CarriedAmmo = CarriedAmmoMap[CurrentWeaponType];
		OnRep_CarriedAmmo();
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::OnRep_CarriedAmmo()
{
	if (Character == nullptr)
	{
		return;
	}

	Controller = Controller == nullptr ? Character->GetController<ABlasterPlayerController>() : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperAmmo);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (!IsValid(EquippedWeapon))
	{
		return;
	}

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomedInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomedInterpSpeed);
	}

	if (IsValid(Character) && IsValid(Character->GetFollowCamera()))
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::SetAiming(const bool bIsAiming)
{
	if (!Character || !EquippedWeapon)
	{
		return;
	}
	
	bAiming = bIsAiming;
	Server_SetAiming(bIsAiming);

	Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}
	
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::SetEquippedWeapon(AWeapon* WeaponToEquip)
{
	if (Character && Character->HasAuthority() && EquippedWeapon != WeaponToEquip)
	{
		EquippedWeapon = WeaponToEquip;
		OnRep_EquippedWeapon();
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::Server_SetAiming_Implementation(const bool bIsAiming)
{
	bAiming = bIsAiming;

	if (IsValid(Character))
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::FireButtonPressed(const bool bPressed)
{
	bFireButtonPressed = bPressed;
	
	if (bFireButtonPressed && IsValid(EquippedWeapon))
	{
		Fire();
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::Fire()
{
	if (CanFire())
	{
		bCanFire = false;
		Server_Fire(HitTarget);
		if (IsValid(EquippedWeapon))
		{
			CrosshairShootingFactor = CrosshairShootAffectValue;
		}

		StartFireTimer();
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::StartFireTimer()
{
	if (!IsValid(EquippedWeapon) || !IsValid(Character))
	{
		return;
	}

	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->FireDelay);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::FireTimerFinished()
{
	if (!IsValid(EquippedWeapon))
	{
		return;
	}

	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bIsAutomatic)
	{
		Fire();
	}

	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
bool UCombatComponent::CanFire()
{
	if (!EquippedWeapon)
	{
		return false;
	}

	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::Server_Fire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	Multicast_Fire(TraceHitTarget);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::Multicast_Fire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (!IsValid(EquippedWeapon))
	{
		return;
	}

	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (IsValid(GEngine) && IsValid(GEngine->GameViewport))
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	const bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		FVector StartLocation = CrosshairWorldPosition;
		
		if (IsValid(Character))
		{
			float DistanceToCharacter = (Character->GetActorLocation() - StartLocation).Size();
			StartLocation += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}

		FVector EndLocation = StartLocation + CrosshairWorldDirection * TraceDistance;

		GetWorld()->LineTraceSingleByChannel(TraceHitResult, StartLocation, EndLocation, ECC_Visibility);

		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = EndLocation;
		}

		HUDPackage.CrosshairsColor = IsValid(TraceHitResult.GetActor()) && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>() ? FLinearColor::Red : FLinearColor::White;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (!IsValid(Character) || !IsValid(WeaponToEquip))
	{
		return;
	}

	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}

	SetEquippedWeapon(WeaponToEquip);
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(HandSocketName);
	if (IsValid(HandSocket))
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		SetCarriedAmmo(EquippedWeapon->GetWeaponType());
	}

	if (EquippedWeapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->EquipSound, Character->GetActorLocation());
	}

	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}

	EquippedWeapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState != ECombatState::ECS_Reloading)
	{
		Server_Reload();
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::Server_Reload_Implementation()
{
	SetCombatState(ECombatState::ECS_Reloading);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::SetCombatState(const ECombatState NewCombatState)
{
	if (Character && Character->HasAuthority() && CombatState != NewCombatState)
	{
		CombatState = NewCombatState;
		OnRep_CombatState();
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::HandleReload()
{
	Character->PlayReloadMontage();
}

//-----------------------------------------------------------------------------------------------------------------------------------
int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr)
	{
		return 0;
	}

	const int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		const int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		const int32 Least = FMath::Min(RoomInMag, AmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}

	return 0;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::FinishReloading()
{
	if (Character == nullptr)
	{
		return;
	}

	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}

	if (bFireButtonPressed)
	{
		Fire();
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr)
	{
		return;
	}

	const int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		SetCarriedAmmo(EquippedWeapon->GetWeaponType());
	}

	EquippedWeapon->AddAmmo(-ReloadAmount);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::OnRep_EquippedWeapon()
{
	if (IsValid(EquippedWeapon) && IsValid(Character))
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(HandSocketName);
		if (IsValid(HandSocket))
		{
			HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		}

		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}

	Controller = !IsValid(Controller) ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller && Character->IsLocallyControlled())
	{
		Controller->SetHUDWeaponIcon(EquippedWeapon ? EquippedWeapon->WeaponIcon : nullptr);
	}
}

