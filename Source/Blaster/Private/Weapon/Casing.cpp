// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Casing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

//-----------------------------------------------------------------------------------------------------------------------------------
ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);

	ShellEjectionImpulse = 5.f;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ACasing::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsValid(CasingMesh))
	{
		CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);
		CasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (IsValid(ShellSound))
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}

	Destroy();
}

