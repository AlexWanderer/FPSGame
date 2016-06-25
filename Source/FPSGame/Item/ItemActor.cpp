// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "ItemActor.h"
#include "Gameplay/UsableActor.h"


AItemActor::AItemActor()
{
	MeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

}

void AItemActor::OnUsed(APawn* UserPawn)
{
	Super::OnUsed(UserPawn);

	OnPickup();

	Destroy();
}

void AItemActor::OnPickup()
{
	if (MeshComp)
	{
		MeshComp->SetVisibility(false);
		MeshComp->SetSimulatePhysics(false);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

