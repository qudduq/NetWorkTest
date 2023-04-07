// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetWorkTestCharacter.h"
#include "NetWorkTestProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/Character.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "Particles/ParticleSystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "NetworkPlayerState.h"
#include "TestSpawnActor.h"
#include "Components/SphereComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ANetWorkTestCharacter

ANetWorkTestCharacter::ANetWorkTestCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	FP_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	FP_Mesh->SetOnlyOwnerSee(true);
	FP_Mesh->SetupAttachment(FirstPersonCameraComponent);
	FP_Mesh->bCastDynamicShadow = false;
	FP_Mesh->CastShadow = false;
	FP_Mesh->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	FP_Mesh->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	FP_Gun->SetupAttachment(FP_Mesh, TEXT("GripPoint"));

	TP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TP_Gun"));
	TP_Gun->SetOwnerNoSee(true);
	TP_Gun->SetupAttachment(GetMesh(), TEXT("hand_rSocket"));


	TP_GunShotParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticlesSysTP"));
	TP_GunShotParticle->bAutoActivate = false;
	TP_GunShotParticle->AttachTo(TP_Gun);
	TP_GunShotParticle->SetOwnerNoSee(true);

	FP_GunShotParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FP_GunShotParticle"));
	FP_GunShotParticle->bAutoActivate = false;
	FP_GunShotParticle->AttachTo(FP_Gun);
	FP_GunShotParticle->SetOnlyOwnerSee(true);

	BulletParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("BulletSysTP"));
	BulletParticle->bAutoActivate = false;
	BulletParticle->AttachTo(FirstPersonCameraComponent);
	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for FP_Mesh, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

}



//////////////////////////////////////////////////////////////////////////
// Input

void ANetWorkTestCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ANetWorkTestCharacter::OnFire);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ANetWorkTestCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ANetWorkTestCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ANetWorkTestCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ANetWorkTestCharacter::LookUpAtRate);
}

void ANetWorkTestCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANetWorkTestCharacter, CurrentTeam);
}

void ANetWorkTestCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	if (GetLocalRole() != ROLE_Authority)
	{
		SetTeam(CurrentTeam);
	}

}

void ANetWorkTestCharacter::SetTeam_Implementation(ETeam NewTeam)
{
	FLinearColor OutColor;
	if(NewTeam == ETeam::BLUE_TEAM)
	{
		OutColor = FLinearColor::Blue;
	}
	else
	{
		OutColor = FLinearColor::Red;
	}

	if(DynamicMat == nullptr)
	{
		DynamicMat = UMaterialInstanceDynamic::Create(GetMesh()->GetMaterial(0), this);
		DynamicMat->SetVectorParameterValue(TEXT("BodyColor"), OutColor);
		GetMesh()->SetMaterial(0, DynamicMat);
		FP_Mesh->SetMaterial(0, DynamicMat);
	}
}

ANetworkPlayerState* ANetWorkTestCharacter::GetNSPlayerState()
{
	if(NSPlayerState != NULL)
	{
		return NSPlayerState;
	}
	else
	{
		NSPlayerState = Cast<ANetworkPlayerState>(GetPlayerState());
		return NSPlayerState;
	}
}

void ANetWorkTestCharacter::SetNSPlayerState(ANetworkPlayerState* newPS)
{
	if(newPS && GetLocalRole() == ROLE_Authority)
	{
		NSPlayerState = newPS;
		SetPlayerState(newPS);
	}
}

void ANetWorkTestCharacter::Respawn()
{
	if(GetLocalRole() == ROLE_Authority)
	{
		NSPlayerState->Health = 100.0f;
		Cast<ANetWorkTestGameMode>(GetWorld()->GetAuthGameMode())->Respawn(this);

		Destroy(true);
	}
}

void ANetWorkTestCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	NSPlayerState = Cast<ANetworkPlayerState>(GetPlayerState());
	if(GetLocalRole() == ROLE_Authority && NSPlayerState != nullptr)
	{
		NSPlayerState->Health = 100.0f;
	}
}


void ANetWorkTestCharacter::OnFire()
{
	if(FP_FireAnimation != NULL)
	{
		UAnimInstance* AnimInstance = FP_Mesh->GetAnimInstance();
		if(AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FP_FireAnimation, 1.0f);
		}
	}

	if(FP_GunShotParticle != nullptr)
	{
		FP_GunShotParticle->Activate(true);
	}

	FVector mousePos;
	FVector mouseDir;

	APlayerController* pController = Cast<APlayerController>(GetController());
	FVector2D ScreenPos = GEngine->GameViewport->Viewport->GetSizeXY();

	pController->DeprojectScreenPositionToWorld(ScreenPos.X / 2.0f, ScreenPos.Y / 2.0f, mousePos, mouseDir);
	mouseDir *= 10000000.0f;

	ServerFire(mousePos, mouseDir);
}

void ANetWorkTestCharacter::ServerFire_Implementation(const FVector Pos, const FVector Dir)
{
	Fire(Pos, Dir);
	MultiShootEffect();
}

bool ANetWorkTestCharacter::ServerFire_Validate(const FVector Pos, const FVector Dir)
{
	if (Pos != FVector(ForceInit) && Dir != FVector(ForceInit))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ANetWorkTestCharacter::Fire(const FVector Pos, const FVector dir)
{
	FCollisionObjectQueryParams ObjectQuery;
	ObjectQuery.AddObjectTypesToQuery(ECC_GameTraceChannel1);

	FCollisionQueryParams ColQuery;
	ColQuery.AddIgnoredActor(this);

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByObjectType(HitResult, Pos, dir, ObjectQuery, ColQuery);

	DrawDebugLine(GetWorld(), Pos, dir, FColor::Red, true, 100, 0, 5.0f);

	AActor* actor = GetWorld()->SpawnActor(TestSpawnClass);

	if(HitResult.bBlockingHit)
	{
		ANetWorkTestCharacter* OtherChar = Cast<ANetWorkTestCharacter>(HitResult.GetActor());
		if(OtherChar != nullptr && OtherChar->GetNSPlayerState()->Team != GetNSPlayerState()->Team)
		{
			FDamageEvent thisEvent(UDamageType::StaticClass());
			OtherChar->TakeDamage(10.0f, thisEvent, GetController(), this);

			APlayerController* thisPC = Cast<APlayerController>(GetController());
			// 클라이언트에 타격에 성공했다고 전해주며 HitSucceFeedback에 저장된 
			thisPC->ClientPlayForceFeedback(HitSuccesFeedback, false, NAME_None);
		}
	}
}


void ANetWorkTestCharacter::MultiShootEffect_Implementation()
{
	if(TP_FireAnimation != NULL)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if(AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(TP_FireAnimation, 1.0f);
		}
	}

	if(TP_GunShotParticle != nullptr)
	{
		TP_GunShotParticle->Activate(true);
	}

	if(BulletParticle != nullptr)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletParticle->Template, BulletParticle->GetComponentLocation(), BulletParticle->GetComponentRotation());
	}
}

float ANetWorkTestCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if(GetLocalRole() == ROLE_Authority && DamageCauser != this && NSPlayerState->Health > 0)
	{
		NSPlayerState->Health -= DamageAmount;
		PlayPain();

		if(NSPlayerState->Health <= 0)
		{
			NSPlayerState->Deaths++;

			MultiCastRagdoll();
			ANetWorkTestCharacter* otherCharacter = Cast<ANetWorkTestCharacter>(DamageCauser);

			if(otherCharacter)
			{
				otherCharacter->NSPlayerState->Score += 1.0f;
			}

			FTimerHandle thisTimer;
			GetWorld()->GetTimerManager().SetTimer<ANetWorkTestCharacter>(thisTimer, this, &ANetWorkTestCharacter::Respawn, 3.0f, false);

		}
	}

	return DamageAmount;
}

void ANetWorkTestCharacter::PlayPain_Implementation()
{
	if(GetLocalRole() == ROLE_Authority)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PainSound,GetActorLocation());
	}
}

void ANetWorkTestCharacter::MultiCastRagdoll_Implementation()
{
	GetMesh()->SetPhysicsBlendWeight(1.0f);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName("Ragdoll");
}


void ANetWorkTestCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ANetWorkTestCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ANetWorkTestCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * 1 * GetWorld()->GetDeltaSeconds());
}

void ANetWorkTestCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * 1 * GetWorld()->GetDeltaSeconds());
}















