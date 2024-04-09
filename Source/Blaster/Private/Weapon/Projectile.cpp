// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Character/BlasterCharacter.h"
#include "Blaster/Blaster.h"
#include "Net/UnrealNetwork.h"

//-----------------------------------------------------------------------------------------------------------------------------------
AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECR_Block);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;

	CurrentHitObject = EHitObject::EHO_None;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsValid(Tracer))
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(Tracer, CollisionBox, NAME_None, GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition);
	}

	if (HasAuthority() && IsValid(CollisionBox))
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//-----------------------------------------------------------------------------------------------------------------------------------
void AProjectile::Destroyed()
{
	Super::Destroyed();

	SpawnHitParticle();

	if (IsValid(ImpactSound))
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Multicast_OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
	Destroy();
}

//-----------------------------------------------------------------------------------------------------------------------------------
void AProjectile::Multicast_OnHit_Implementation(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (IsValid(BlasterCharacter))
	{
		CurrentHitObject = EHitObject::EHO_Character;
		BlasterCharacter->PlayHitReactMontage();
	}
	else
	{
		CurrentHitObject = EHitObject::EHO_Surface;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void AProjectile::SpawnHitParticle()
{
	UParticleSystem* SelectedParticleSystem = nullptr;
	switch (CurrentHitObject)
	{
	case EHitObject::EHO_Character:
		SelectedParticleSystem = CharacterImpactParticles;
		break;
	case EHitObject::EHO_Surface:
		SelectedParticleSystem = ImpactParticles;
		break;
	default:
		break;
	}

	if (IsValid(SelectedParticleSystem))
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedParticleSystem, GetActorTransform());
	}
}

