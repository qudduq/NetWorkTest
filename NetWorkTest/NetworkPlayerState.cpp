// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkPlayerState.h"

#include "Net/UnrealNetwork.h"

ANetworkPlayerState::ANetworkPlayerState(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	Health = 100.0f;
	Deaths = 0;
	Team = ETeam::BLUE_TEAM;
}

void ANetworkPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANetworkPlayerState, Health);
	DOREPLIFETIME(ANetworkPlayerState, Deaths);
	DOREPLIFETIME(ANetworkPlayerState, Team);
}
