// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/SpeedPickup.h"

#include "MaterialHLSLTree.h"
#include "BlasterComponents/BuffComponent.h"
#include "Character/BlasterCharacter.h"

//-----------------------------------------------------------------------------------------------------------------------------------
ASpeedPickup::ASpeedPickup()
{
	PrimaryActorTick.bCanEverTick = true;
	
	BaseSpeedBuff = 1600.f;
	CrouchSpeedBuff = 850.f;
	SpeedBuffTime = 30.f;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ASpeedPickup::BeginPlay()
{
	Super::BeginPlay();
	
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ASpeedPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ASpeedPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	
	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
	{
		if (UBuffComponent* BuffComponent = BlasterCharacter->GetBuffComponent())
		{
			BuffComponent->BuffSpeed(BaseSpeedBuff, CrouchSpeedBuff, SpeedBuffTime);
		}
	}

	Destroy();
}


