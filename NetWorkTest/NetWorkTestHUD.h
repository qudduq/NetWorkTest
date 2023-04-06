// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "NetWorkTestHUD.generated.h"

UCLASS()
class ANetWorkTestHUD : public AHUD
{
	GENERATED_BODY()

public:
	ANetWorkTestHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

