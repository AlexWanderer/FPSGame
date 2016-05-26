// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "PickupActor.h"
#include "Player/FPSCharacter.h"
#include "Online/FPSGameMode.h"

APickupActor::APickupActor()
{
	UCapsuleComponent* CollisionComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionComp"));
	CollisionComp->InitCapsuleSize(40.0f, 50.0f);
	CollisionComp->SetCollisionObjectType(COLLISION_PICKUP);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = CollisionComp;

	PickupPSC = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("PickupFX"));
	PickupPSC->bAutoActivate = false;
	PickupPSC->bAutoDestroy = false;
	PickupPSC->AttachParent = RootComponent;

	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;

	PickedUpBy = NULL;
	bIsActive = false;
	RespawnTime = 10.0f;
}


void APickupActor::BeginPlay()
{
	Super::BeginPlay();

	RespawnPickup();

	// register on pickup list (server only), don't care about unregistering (in FinishDestroy) - no streaming
	AFPSGameMode* GameMode = GetWorld()->GetAuthGameMode<AFPSGameMode>();
	if (GameMode)
	{
		GameMode->LevelPickups.Add(this);
	}
}

void APickupActor::NotifyActorBeginOverlap(class AActor* Other)
{
	Super::NotifyActorBeginOverlap(Other);
	PickupOnTouch(Cast<AFPSCharacter>(Other));
}

void APickupActor::OnRep_IsActive()
{
	if (bIsActive)
	{
		OnRespawned();
	}
	else
	{
		OnPickedUp();
	}
}

void APickupActor::PickupOnTouch(class AFPSCharacter* Pawn)
{
	if (bIsActive && Pawn && Pawn->IsAlive() && !IsPendingKill())
	{
		if (CanBePickedUp(Pawn))
		{
			GivePickupTo(Pawn);
			PickedUpBy = Pawn;

			if (!IsPendingKill())
			{
				bIsActive = false;
				OnPickedUp();

				if (RespawnTime > 0.0f)
				{
					GetWorldTimerManager().SetTimer(TimerHandle_RespawnPickup, this, &APickupActor::RespawnPickup, RespawnTime, false);
				}
			}
		}
	}
}

void APickupActor::OnPickedUp()
{
	if (RespawningFX)
	{
		PickupPSC->SetTemplate(RespawningFX);
		PickupPSC->ActivateSystem();
	}
	else
	{
		PickupPSC->DeactivateSystem();
	}

	if (PickupSound && PickedUpBy)
	{
		UGameplayStatics::SpawnSoundAttached(PickupSound, PickedUpBy->GetRootComponent());
	}

	OnPickedUpEvent();
}

bool APickupActor::CanBePickedUp(class AFPSCharacter* TestPawn) const
{
	return TestPawn && TestPawn->IsAlive();
}

void APickupActor::GivePickupTo(class AFPSCharacter* Pawn)
{

}

void APickupActor::RespawnPickup()
{
	bIsActive = true;
	PickedUpBy = NULL;
	OnRespawned();

	TArray<AActor*> OverlappingPawns;
	GetOverlappingActors(OverlappingPawns, AFPSCharacter::StaticClass());

	for (int32 i = 0; i < OverlappingPawns.Num(); i++)
	{
		PickupOnTouch(Cast<AFPSCharacter>(OverlappingPawns[i]));
	}
}

void APickupActor::OnRespawned()
{
	if (ActiveFX)
	{
		PickupPSC->SetTemplate(ActiveFX);
		PickupPSC->ActivateSystem();
	}
	else
	{
		PickupPSC->DeactivateSystem();
	}

	const bool bJustSpawned = CreationTime <= (GetWorld()->GetTimeSeconds() + 5.0f);
	if (RespawnSound && !bJustSpawned)
	{
		UGameplayStatics::PlaySoundAtLocation(this, RespawnSound, GetActorLocation());
	}

	OnRespawnEvent();

}

void APickupActor::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APickupActor, bIsActive);
	DOREPLIFETIME(APickupActor, PickedUpBy);
}