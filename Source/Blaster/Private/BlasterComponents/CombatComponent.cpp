// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/CombatComponent.h"
#include "Weapon/Weapon.h"
#include "Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"

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
}

