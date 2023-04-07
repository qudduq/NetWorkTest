// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetWorkTestGameMode.h"
#include "NetWorkTestHUD.h"
#include "NetWorkTestCharacter.h"
#include "NetworkPlayerState.h"
#include "NSSpawnPoint.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"

bool ANetWorkTestGameMode::bTnGameMenu = true;

ANetWorkTestGameMode::ANetWorkTestGameMode()
	:Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
	PlayerStateClass = ANetworkPlayerState::StaticClass();
	bReplicates = true;

	// use our custom HUD class
	HUDClass = ANetWorkTestHUD::StaticClass();
}

void ANetWorkTestGameMode::BeginPlay()
{
	Super::BeginPlay();
	if(GetLocalRole() == ROLE_Authority)
	{
		for(TActorIterator<ANSSpawnPoint> Iterator(GetWorld()); Iterator; ++Iterator )
		{
			if((*Iterator)->Team == ETeam::RED_TEAM)
			{
				RedSpawns.Add(*Iterator);
			}
			else
			{
				BlueSpawns.Add(*Iterator);
			}
		}
	}

	//서버스폰
	APlayerController* ThisController = GetWorld()->GetFirstPlayerController();
	if(ThisController)
	{
		ANetWorkTestCharacter* ThisCharacter = Cast<ANetWorkTestCharacter>(ThisController->GetPawn());
		ThisCharacter->SetTeam(ETeam::BLUE_TEAM);
		BlueTeam.Add(ThisCharacter);
		Spawn(ThisCharacter);
	}
}

void ANetWorkTestGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(GetLocalRole() == ROLE_Authority)
	{
		APlayerController* ThisController = GetWorld()->GetFirstPlayerController();
		if(ToBeSPawnd.Num()!=0)
		{
			for(auto charToSpawn : ToBeSPawnd)
			{
				Spawn(charToSpawn);
			}
		}

		if(ThisController != nullptr && ThisController->IsInputKeyDown(EKeys::R))
		{
			bTnGameMenu = false;
			//World'/Game/FirstPersonCPP/Maps/FirstPersonExampleMap.FirstPersonExampleMap'
			// ?Listen 이 붙은 이유 : 서버가 이동할 맵이 클라이언트의 연결을 허용하는지 확인하는 추가 명령
			GetWorld()->ServerTravel(L"/Game/FirstPersonCPP/Maps/FirstPersonExampleMap?Listen");
		}
	}
}

void ANetWorkTestGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ANetWorkTestCharacter* Teamless = Cast<ANetWorkTestCharacter>(NewPlayer->GetPawn());
	ANetworkPlayerState* NSPlayerState = NewPlayer->GetPlayerState<ANetworkPlayerState>();

	if (Teamless != nullptr && NSPlayerState != nullptr)
	{
		Teamless->SetNSPlayerState(NSPlayerState);
	}

	if (GetLocalRole() == ROLE_Authority && Teamless != nullptr)
	{
		if (BlueTeam.Num() > RedTeam.Num())
		{
			RedTeam.Add(Teamless);
			NSPlayerState->Team = ETeam::RED_TEAM;
		}
		else if(BlueTeam.Num() <= RedTeam.Num())
		{
			BlueTeam.Add(Teamless);
			NSPlayerState->Team = ETeam::BLUE_TEAM;
		}

		Teamless->CurrentTeam = NSPlayerState->Team;
		Teamless->SetTeam(NSPlayerState->Team);
		Spawn(Teamless);
	}
}

void ANetWorkTestGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if(EndPlayReason == EEndPlayReason::Quit || EndPlayReason == EEndPlayReason::EndPlayInEditor)
	{
		bTnGameMenu = true;
	}
}

void ANetWorkTestGameMode::Respawn(ANetWorkTestCharacter* Character)
{
	if(GetLocalRole() == ROLE_Authority)
	{
		AController* thisPC = Character->GetController();
		Character->DetachFromControllerPendingDestroy();

		ANetWorkTestCharacter* NewCharacter = Cast<ANetWorkTestCharacter>(GetWorld()->SpawnActor(DefaultPawnClass));

		if(NewCharacter)
		{
			thisPC->Possess(NewCharacter);
			ANetworkPlayerState* thisPS = NewCharacter->GetController()->GetPlayerState<ANetworkPlayerState>();

			NewCharacter->CurrentTeam = thisPS->Team;
			NewCharacter->SetNSPlayerState(thisPS);

			Spawn(NewCharacter);

			NewCharacter->SetTeam(NewCharacter->GetNSPlayerState()->Team);
		}
	}
}

void ANetWorkTestGameMode::Spawn(ANetWorkTestCharacter* Character)
{
	if(ROLE_Authority == GetLocalRole())
	{
		ANSSpawnPoint* thisSpawn = nullptr;
		TArray<ANSSpawnPoint*>* TargetTeam = nullptr;

		if(Character->CurrentTeam == ETeam::BLUE_TEAM)
		{
			TargetTeam = &BlueSpawns;
		}
		else
		{
			TargetTeam = &RedSpawns;
		}

		for(auto Spawn : (*TargetTeam))
		{
			if(!Spawn->Getblocked())
			{
				if(ToBeSPawnd.Find(Character) != INDEX_NONE)
				{
					ToBeSPawnd.Remove(Character);
				}

				Character->SetActorLocation(Spawn->GetActorLocation());
				Spawn->UpdateOverlaps();
				return;
			}

			if(ToBeSPawnd.Find(Character) == INDEX_NONE)
			{
				ToBeSPawnd.Add(Character);
			}
		}
	}
}
