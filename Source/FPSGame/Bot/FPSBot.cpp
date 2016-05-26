// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "FPSBot.h"
#include "Bot/FPSAIController.h"

AFPSBot::AFPSBot()
	: Super()
{
	AIControllerClass = AFPSAIController::StaticClass();

	UpdatePawnMeshes();

	bUseControllerRotationYaw = true;
}


bool AFPSBot::IsFirstPerson() const
{
	return false;
}

void AFPSBot::FaceRotation(FRotator NewRotation, float DeltaTime /*= 0.f*/)
{
	FRotator CurrentRotation = FMath::RInterpTo(GetActorRotation(), NewRotation, DeltaTime, 8.0f);

	Super::FaceRotation(CurrentRotation, DeltaTime);
}
