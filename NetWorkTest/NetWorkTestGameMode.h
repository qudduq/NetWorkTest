// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetWorkTestGameMode.generated.h"

class ANetWorkTestCharacter;
class ANSSpawnPoint;

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

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void Respawn(ANetWorkTestCharacter* Character);
	void Spawn(ANetWorkTestCharacter* Character);

private:
	TArray<ANetWorkTestCharacter*> RedTeam;
	TArray<ANetWorkTestCharacter*> BlueTeam;

	TArray<ANSSpawnPoint*> RedSpawns;
	TArray<ANSSpawnPoint*> BlueSpawns;
	TArray<ANetWorkTestCharacter*> ToBeSPawnd;

	bool bGameStarted;
	static bool bTnGameMenu;
};



