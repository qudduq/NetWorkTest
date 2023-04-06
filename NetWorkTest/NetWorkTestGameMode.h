// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetWorkTestGameMode.generated.h"

UENUM(BlueprintType)
enum class ETeam : uint8
{
	BLUE_TEAM,
	RED_TEAM
};

UCLASS(minimalapi)
class ANetWorkTestGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANetWorkTestGameMode();
};



