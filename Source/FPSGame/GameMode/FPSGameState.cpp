// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "FPSGameState.h"


AFPSGameState::AFPSGameState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NumTeams = 0;
	RemainingTime = 0;
	bTimerPaused = false;
}