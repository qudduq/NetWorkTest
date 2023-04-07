// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NSSpawnPoint.generated.h"

enum class ETeam : uint8;
class UCapsuleComponent;
UCLASS()
class NETWORKTEST_API ANSSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANSSpawnPoint();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION()
		void ActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
		void ActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

	bool Getblocked()
	{
		return OverlappingActors.Num() != 0;
	}

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		ETeam Team;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UCapsuleComponent* SpawnCapsule;
	TArray<class AActor*> OverlappingActors;

};
