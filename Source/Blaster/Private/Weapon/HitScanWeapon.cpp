// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeapon.h"

#include "Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"


//-----------------------------------------------------------------------------------------------------------------------------------
AHitScanWeapon::AHitScanWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	FlashSocketName = "MuzzleFlash";
	Damage = 20.f;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void AHitScanWeapon::BeginPlay()
{
	Super::BeginPlay();
	
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
		FVector End = Start + (HitTarget - Start) * 1.25f;

		FHitResult FireHit;
		if (GetWorld())
		{
			GetWorld()->LineTraceSingleByChannel(FireHit, Start, End, ECC_Visibility);

			FVector BeamEnd = End;
			if (FireHit.bBlockingHit)
			{
				BeamEnd = FireHit.ImpactPoint;
				
				// Make damage only on server
				AController* InstigatorController = OwnerPawn->GetController();
				ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
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
			}

			if (BeamParticles)
			{
				UParticleSystemComponent* BeamComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
					BeamParticles, SocketTransform);
				if (BeamComponent)
				{
					BeamComponent->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
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



