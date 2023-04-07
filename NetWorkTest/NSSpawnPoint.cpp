// Fill out your copyright notice in the Description page of Project Settings.


#include "NSSpawnPoint.h"

#include "Components/CapsuleComponent.h"
#include "NetWorkTestGameMode.h"

// Sets default values
ANSSpawnPoint::ANSSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;

	SpawnCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	SpawnCapsule->SetCollisionProfileName("OverlapAllDynamic");
	SpawnCapsule->SetGenerateOverlapEvents(true);
	SpawnCapsule->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap); // ?

	OnActorBeginOverlap.AddDynamic(this, &ANSSpawnPoint::ActorBeginOverlap);
	OnActorEndOverlap.AddDynamic(this, &ANSSpawnPoint::ActorEndOverlap);

}

// Called when the game starts or when spawned
void ANSSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	
}

void ANSSpawnPoint::OnConstruction(const FTransform& Transform)
{
	if(Team == ETeam::RED_TEAM)
	{
		SpawnCapsule->ShapeColor = FColor::Red;
	}
	else // Team == ETeam::BLUE_TEAM
	{
		SpawnCapsule->ShapeColor = FColor::Blue;
	}


}

void ANSSpawnPoint::ActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if(ROLE_Authority == GetLocalRole())
	{
		if(OverlappingActors.Find(OtherActor) == INDEX_NONE)
		{
			OverlappingActors.Add(OtherActor);
		}
	}
}

void ANSSpawnPoint::ActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if(ROLE_Authority == GetLocalRole())
	{
		if(OverlappingActors.Find(OtherActor))
		{
			OverlappingActors.Remove(OtherActor);
		}
	}
}

// Called every frame
void ANSSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SpawnCapsule->UpdateOverlaps();
}

