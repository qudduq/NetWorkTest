// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetWorkTestGameMode.h"
#include "NetWorkTestHUD.h"
#include "NetWorkTestCharacter.h"
#include "UObject/ConstructorHelpers.h"

ANetWorkTestGameMode::ANetWorkTestGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ANetWorkTestHUD::StaticClass();
}
