// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Character/BlasterCharacter.h"
#include "Blaster/Blaster.h"

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

	DestroyTime = 3.f;
	CurrentHitObject = EHitObject::EHO_None;

	Damage = 20.f;
	DamageInnerRadius = 200.f;
	DamageOuterRadius = 500.f;
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
void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AProjectile::DestroyTimerFinished, DestroyTime);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void AProjectile::DestroyTimerFinished()
{
	Destroy();
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

	SpawnHitEffects();
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

//-----------------------------------------------------------------------------------------------------------------------------------
void AProjectile::SpawnHitEffects()
{
	SpawnHitParticle();

	if (IsValid(ImpactSound))
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void AProjectile::SpawnTrailSystem()
{
	if (TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(TrailSystem, GetRootComponent(), FName(), GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition, false);
	}

}

//-----------------------------------------------------------------------------------------------------------------------------------
void AProjectile::ExplodeDamage()
{
	const APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority())
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(this, Damage, 10.f, GetActorLocation(),
				DamageInnerRadius, DamageOuterRadius, 1.f, UDamageType::StaticClass(), TArray<AActor*>(), this, FiringController);
		}
	}
}

