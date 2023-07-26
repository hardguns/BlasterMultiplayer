// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BlasterAnimInstance.h"
#include "Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

//-----------------------------------------------------------------------------------------------------------------------------------
void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (IsValid(BlasterCharacter))
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}

	if (!IsValid(BlasterCharacter))
	{
		return;
	}

	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();

	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
}
