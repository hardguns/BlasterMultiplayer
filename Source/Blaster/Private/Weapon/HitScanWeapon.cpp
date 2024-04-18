// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeapon.h"

#include "Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/WeaponTypes.h"

//-----------------------------------------------------------------------------------------------------------------------------------
AHitScanWeapon::AHitScanWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	FlashSocketName = "MuzzleFlash";
	Damage = 20.f;

	//Scatter
	DistanceToSphere = 800.f;
	SphereRadius = 75.f;
	bUseScatter = false;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void AHitScanWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

//-----------------------------------------------------------------------------------------------------------------------------------
FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	const FVector RandomVector = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(0.f, SphereRadius);
	const FVector EndLocation = SphereCenter + RandomVector;
	FVector DistanceToEndLocation = EndLocation - TraceStart;

	// DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	// DrawDebugSphere(GetWorld(), EndLocation, 4.f, 12, FColor::Blue, true);
	
	const FVector NewTraceEnd = FVector(TraceStart + DistanceToEndLocation * TRACE_LENGTH / DistanceToEndLocation.Size());
	// DrawDebugLine(GetWorld(), TraceStart,NewTraceEnd, FColor::Orange, true);
	
	return NewTraceEnd;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FlashSocketName);
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);
		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());

		// Make damage only on server
		AController* InstigatorController = OwnerPawn->GetController();
		if (BlasterCharacter && HasAuthority() && InstigatorController)
		{
			UGameplayStatics::ApplyDamage(BlasterCharacter, Damage, InstigatorController, this, UDamageType::StaticClass());
		}

		if (ImpactParticles)
		{
			// Spawn particle when hit
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
		}

		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
		}

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	if (GetWorld())
	{
		const FVector End = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;
		GetWorld()->LineTraceSingleByChannel(OutHit, TraceStart, End, ECC_Visibility);

		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}

		if (BeamParticles)
		{
			UParticleSystemComponent* BeamComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
				BeamParticles, TraceStart, FRotator::ZeroRotator, true);
			if (BeamComponent)
			{
				BeamComponent->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}


