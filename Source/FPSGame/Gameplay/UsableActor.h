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

	UFUNCTION(BlueprintImplementableEvent, Category = UsableActor)
	void ReceiveUsed(APawn* UserPawn);

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	USkeletalMeshComponent* SkeletalMeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	UStaticMeshComponent* StaticMeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	bool bStaticMesh = false;


	FORCEINLINE UMeshComponent* GetMeshComponent() const
	{
		if (bStaticMesh)
		{
			return StaticMeshComp;
		}
		else
		{
			return SkeletalMeshComp;
		}
		
	}
};
