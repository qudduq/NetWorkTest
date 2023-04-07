// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NetWorkTestGameMode.h"
#include "GameFramework/Character.h"
#include "NetWorkTestCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UMotionControllerComponent;
class UAnimMontage;
class USoundBase;

UCLASS(config=Game)
class ANetWorkTestCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* FP_Mesh;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* FP_Gun;

	/** Gun mesh: ¿ÜºÎ»ç¶÷ÀÌ º¸´Â ÃÑ */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		USkeletalMeshComponent* TP_Gun;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** Motion controller (right hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UMotionControllerComponent* R_MotionController;

	/** Motion controller (left hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UMotionControllerComponent* L_MotionController;

public:
	ANetWorkTestCharacter();

public:
	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
		USoundBase* FireSound;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		USoundBase* PainSound;

	/** AnimMontage to play each time we fire 3ÀÎÄª*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		UAnimMontage* TP_FireAnimation;

	/** AnimMontage to play each time we fire 1ÀÎÄª*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		UAnimMontage* FP_FireAnimation;

	// ÆÄÆ¼Å¬ 1ÀÎÄª
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		UParticleSystemComponent* FP_GunShotParticle;

	// ÆÄÆ¼Å¬ 3ÀÎÄª
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		UParticleSystemComponent* TP_GunShotParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		UParticleSystemComponent* BulletParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		UForceFeedbackEffect* HitSuccesFeedback;

	UPROPERTY(EditAnywhere)
		TSubclassOf<class ATestSpawnActor> TestSpawnClass;

public:
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Team")
		ETeam CurrentTeam;

protected:
	class UMaterialInstanceDynamic* DynamicMat;
	class ANetworkPlayerState* NSPlayerState;

public:
	class ANetworkPlayerState* GetNSPlayerState();
	void SetNSPlayerState(class ANetworkPlayerState* newPS);
	void Respawn();

protected:
	void OnFire();
	void MoveForward(float Val);
	void MoveRight(float Val);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	void Fire(const FVector Pos, const FVector dir);
protected:
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

private:
	UFUNCTION(Reliable, Server, WithValidation)
		void ServerFire(const FVector Pos, const FVector Dir);

	UFUNCTION(NetMulticast, Unreliable)
		void MultiShootEffect();

	UFUNCTION(NetMulticast, Unreliable)
		void MultiCastRagdoll();

	UFUNCTION(Client, Reliable)
		void PlayPain();

public:
	UFUNCTION(NetMulticast, Reliable)
		void SetTeam(ETeam NewTeam);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetFPMesh() const { return FP_Mesh; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

