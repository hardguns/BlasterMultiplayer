// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/HealthPickup.h"

#include "BlasterComponents/BuffComponent.h"
#include "Character/BlasterCharacter.h"

//-----------------------------------------------------------------------------------------------------------------------------------
AHealthPickup::AHealthPickup()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
	
	HealAmount = 100.f;
	HealingTime = 5.f;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void AHealthPickup::BeginPlay()
{
	Super::BeginPlay();
	
}

//-----------------------------------------------------------------------------------------------------------------------------------
void AHealthPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	
	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
	{
		if (UBuffComponent* BuffComponent = BlasterCharacter->GetBuffComponent())
		{
			BuffComponent->Heal(HealAmount, HealingTime);
		}
	}

	Destroy();
}