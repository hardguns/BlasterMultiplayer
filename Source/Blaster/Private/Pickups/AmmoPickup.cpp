// Fill out your copyright notice in the Description page of Project Settings.

#include "Pickups/AmmoPickup.h"

#include "BlasterComponents/CombatComponent.h"
#include "Character/BlasterCharacter.h"

//-----------------------------------------------------------------------------------------------------------------------------------
AAmmoPickup::AAmmoPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	AmmoAmount = 30;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter && BlasterCharacter->GetCombatComponent())
	{
		BlasterCharacter->GetCombatComponent()->PickupAmmo(WeaponType, AmmoAmount);
	}

	Destroy();
}
