// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "UsableActor.h"


// Sets default values
AUsableActor::AUsableActor() : Super()
{

	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	RootComponent = SkeletalMeshComp;
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Meshs"));
	StaticMeshComp->AttachParent = RootComponent;
	

}

void AUsableActor::OnBeginFocus()
{
	if (bStaticMesh)
	{
		if (StaticMeshComp)
		{
			StaticMeshComp->SetRenderCustomDepth(true);
		}
	}
	else
	{
		if (SkeletalMeshComp)
		{
			SkeletalMeshComp->SetRenderCustomDepth(true);
		}
	}
}

void AUsableActor::OnEndFocus()
{
	if (bStaticMesh)
	{
		if (StaticMeshComp)
		{
			StaticMeshComp->SetRenderCustomDepth(false);
		}
	}
	else
	{
		if (SkeletalMeshComp)
		{
			SkeletalMeshComp->SetRenderCustomDepth(false);
		}
	}
}

void AUsableActor::OnUsed(APawn* UserPawn)
{
	ReceiveUsed(UserPawn);
}

