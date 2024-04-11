// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeapon.h"


// Sets default values
AHitScanWeapon::AHitScanWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AHitScanWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHitScanWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

