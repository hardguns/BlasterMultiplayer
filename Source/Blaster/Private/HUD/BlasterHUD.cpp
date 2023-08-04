// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/BlasterHUD.h"

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		DrawTextureCrosshairs(ViewportCenter);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterHUD::DrawTextureCrosshairs(const FVector2D& ViewportCenter)
{
	if (IsValid(HUDPackage.CrosshairsCenter))
	{
		DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter);
	}

	if (IsValid(HUDPackage.CrosshairsLeft))
	{
		DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter);
	}

	if (IsValid(HUDPackage.CrosshairsRight))
	{
		DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter);
	}

	if (IsValid(HUDPackage.CrosshairsTop))
	{
		DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter);
	}

	if (IsValid(HUDPackage.CrosshairsBottom))
	{
		DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(ViewportCenter.X - (TextureWidth / 2.f), ViewportCenter.Y - (TextureHeight / 2.f));

	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight, 0.f, 0.f, 1.f, 1.f, FLinearColor::White);
}
