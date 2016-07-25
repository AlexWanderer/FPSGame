// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "FPSBot.h"
#include "Bot/FPSAIController.h"
#include "Player/FPSPlayerState.h"

AFPSBot::AFPSBot()
	: Super()
{
	AIControllerClass = AFPSAIController::StaticClass();
	

	bUseControllerRotationYaw = true;

}


bool AFPSBot::IsFirstPerson() const
{
	return false;
}

void AFPSBot::BeginPlay()
{
	Super::BeginPlay();
	UpdatePawnMeshes();
	AFPSPlayerState* PS = Cast<AFPSPlayerState>(PlayerState);
	if (PS)
	{
		PS->SetPlayerName("Bot");
		PS->bIsABot = true;
	}
}

void AFPSBot::FaceRotation(FRotator NewRotation, float DeltaTime /*= 0.f*/)
{
	FRotator CurrentRotation = FMath::RInterpTo(GetActorRotation(), NewRotation, DeltaTime, 8.0f);

	Super::FaceRotation(CurrentRotation, DeltaTime);
}
