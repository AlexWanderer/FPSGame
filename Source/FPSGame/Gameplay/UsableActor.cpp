// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "UsableActor.h"


// Sets default values
AUsableActor::AUsableActor() : Super()
{

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComp;
}

void AUsableActor::OnBeginFocus()
{
	if (MeshComp)
	{
		MeshComp->SetRenderCustomDepth(true);
	}
}

void AUsableActor::OnEndFocus()
{
	if (MeshComp)
	{
		MeshComp->SetRenderCustomDepth(false);
	}
}

void AUsableActor::OnUsed(APawn* UserPawn)
{

}

