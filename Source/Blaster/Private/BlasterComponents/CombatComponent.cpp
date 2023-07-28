// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/CombatComponent.h"
#include "Weapon/Weapon.h"
#include "Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

//-----------------------------------------------------------------------------------------------------------------------------------
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	HandSocketName = "RightHandSocket";
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::SetAiming(const bool bIsAiming)
{
	bAiming = bIsAiming;
	Server_SetAiming(bIsAiming);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UCombatComponent::Server_SetAiming_Implementation(const bool bIsAiming)
{
	bAiming = bIsAiming;
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

