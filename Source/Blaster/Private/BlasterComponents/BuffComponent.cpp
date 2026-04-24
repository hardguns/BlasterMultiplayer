// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/BuffComponent.h"
#include "Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

//-----------------------------------------------------------------------------------------------------------------------------------
UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	bIsHealing = false;
	HealingRate = 0.f;
	AmountToHeal = 0.f;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

}

//-----------------------------------------------------------------------------------------------------------------------------------
void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	HealRampUp(DeltaTime);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UBuffComponent::Heal(const float HealAmount, const float HealingTime)
{
	bIsHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UBuffComponent::HealRampUp(const float DeltaTime)
{
	if (!bIsHealing || !Character || Character->IsElimmed())
	{
		return;
	}
	
	const float HealThisFrame = HealingRate * DeltaTime;
	AmountToHeal -= HealThisFrame;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));

	if (AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bIsHealing = false;	
		AmountToHeal = 0.f;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UBuffComponent::BuffSpeed(const float BuffBaseSpeed, float BuffCrouchSpeed, const float BuffTime)
{
	if (!Character)
	{
		return;
	}
	
	Character->GetWorldTimerManager().SetTimer(SpeedBuffTimerHandle, this, &UBuffComponent::ResetSpeeds, BuffTime);

	Multicast_SpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UBuffComponent::SetInitialSpeeds(const float BaseSpeed, const float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UBuffComponent::ResetSpeeds()
{
	if (!Character || !Character->GetCharacterMovement())
	{
		return;
	}
	
	Multicast_SpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void UBuffComponent::Multicast_SpeedBuff_Implementation(const float BaseSpeed, const float CrouchSpeed)
{
	if (!Character || !Character->GetCharacterMovement())
	{
		return;
	}
	
	Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
}

