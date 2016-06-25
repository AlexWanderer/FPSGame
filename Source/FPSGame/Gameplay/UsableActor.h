// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "UsableActor.generated.h"

UCLASS()
class FPSGAME_API AUsableActor : public AActor
{
	GENERATED_BODY()
	
public:	

	AUsableActor();

	virtual void OnBeginFocus();

	virtual void OnEndFocus();

	virtual void OnUsed(APawn* UserPawn);

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	USkeletalMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	UBoxComponent* TraceBox;

	FORCEINLINE USkeletalMeshComponent* GetMeshComponent() const { return MeshComp; }
};
