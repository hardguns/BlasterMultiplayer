// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/BlasterHUD.h"
#include "GameFramework/PlayerController.h"
#include "HUD/CharacterOverlay.h"

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();

	AddCharacterOverlay();
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (IsValid(PlayerController) && IsValid(CharacterOverlayClass))
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		if (IsValid(CharacterOverlay))
		{
			CharacterOverlay->AddToViewport();
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		DrawTextureCrosshairs(ViewportCenter, SpreadScaled);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterHUD::DrawTextureCrosshairs(const FVector2D& ViewportCenter, const float SpreadScaled)
{
	if (IsValid(HUDPackage.CrosshairsCenter))
	{
		FVector2D Spread(0.f, 0.f);
		DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
	}

	if (IsValid(HUDPackage.CrosshairsLeft))
	{
		FVector2D Spread(-SpreadScaled, 0.f);
		DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
	}

	if (IsValid(HUDPackage.CrosshairsRight))
	{
		FVector2D Spread(SpreadScaled, 0.f);
		DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
	}

	if (IsValid(HUDPackage.CrosshairsTop))
	{
		FVector2D Spread(0.f, -SpreadScaled);
		DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
	}

	if (IsValid(HUDPackage.CrosshairsBottom))
	{
		FVector2D Spread(0.f, SpreadScaled);
		DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(ViewportCenter.X - (TextureWidth / 2.f) + Spread.X, 
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y);

	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight, 0.f, 0.f, 1.f, 1.f, CrosshairColor);
}
