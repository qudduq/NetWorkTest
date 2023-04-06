// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetWorkTestGameMode.h"
#include "GameFramework/PlayerState.h"
#include "NetworkPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKTEST_API ANetworkPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ANetworkPlayerState(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(Replicated)
		float Health;

	UPROPERTY(Replicated)
		uint8 Deaths;

	UPROPERTY(Replicated)
		ETeam Team;
};
