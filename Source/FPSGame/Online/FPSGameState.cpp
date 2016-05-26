// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "FPSGameState.h"


AFPSGameState::AFPSGameState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NumTeams = 0;
	RemainingTime = 0;
	bTimerPaused = false;
}


void AFPSGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSGameState, NumTeams);
	DOREPLIFETIME(AFPSGameState, RemainingTime);
	DOREPLIFETIME(AFPSGameState, bTimerPaused);
	DOREPLIFETIME(AFPSGameState, TeamScores);
}