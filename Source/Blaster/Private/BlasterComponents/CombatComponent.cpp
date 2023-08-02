// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/CombatComponent.h"
#include "Weapon/Weapon.h"
#include "Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

//-----------------------------------------------------------------------------------------------------------------------------------
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	HandSocketName = "RightHandSocket";

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;

	TraceDistance = 80000.f;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(Character))
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FHitResult HitResult;
	TraceUnderCrosshairs(HitResult);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::SetAiming(const bool bIsAiming)
{
	bAiming = bIsAiming;
	Server_SetAiming(bIsAiming);

	if (IsValid(Character))
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
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
void UCombatComponent::OnRep_EquippedWeapon()
{
	if (IsValid(EquippedWeapon) && IsValid(Character))
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::FireButtonPressed(const bool bPressed)
{
	bFireButtonPressed = bPressed;
	
	if (bFireButtonPressed)
	{
		Server_Fire();
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::Server_Fire_Implementation()
{
	Multicast_Fire();
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::Multicast_Fire_Implementation()
{
	if (!IsValid(EquippedWeapon))
	{
		return;
	}

	if (IsValid(Character))
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(HitTarget);
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
		const FVector StartLocation = CrosshairWorldPosition;
		FVector EndLocation = StartLocation + CrosshairWorldDirection * TraceDistance;

		GetWorld()->LineTraceSingleByChannel(TraceHitResult, StartLocation, EndLocation, ECC_Visibility);

		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = EndLocation;
			HitTarget = EndLocation;
		}
		else
		{
			HitTarget = TraceHitResult.ImpactPoint;
			DrawDebugSphere(GetWorld(), TraceHitResult.ImpactPoint, 12.f, 12, FColor::Red);
		}
	}

}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (!IsValid(Character) || !IsValid(WeaponToEquip))
	{
		return;
	}

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(HandSocketName);
	if (IsValid(HandSocket))
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}

	EquippedWeapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

