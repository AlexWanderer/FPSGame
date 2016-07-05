// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Player/FPSCharacter.h"
#include "FPSBot.generated.h"

UCLASS()
class AFPSBot : public AFPSCharacter
{
	GENERATED_BODY()

	AFPSBot();

public:
	UPROPERTY(EditAnywhere, Category = Behavior)
	class UBehaviorTree* BotBehavior;

	virtual bool IsFirstPerson() const override;

	virtual void FaceRotation(FRotator NewRotation, float DeltaTime = 0.f) override;
	
};
