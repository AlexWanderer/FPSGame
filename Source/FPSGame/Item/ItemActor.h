// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ItemActor.generated.h"

class AUsableActor;

UCLASS()
class FPSGAME_API AItemActor : public AUsableActor
{
	GENERATED_BODY()
	
public:	

	AItemActor();

	virtual void OnUsed(APawn* UserPawn) override;

	virtual void OnPickup();

};