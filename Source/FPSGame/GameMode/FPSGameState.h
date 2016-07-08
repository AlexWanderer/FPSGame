// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameState.h"
#include "FPSGameState.generated.h"

/**
 * 
 */
UCLASS()
class AFPSGameState : public AGameState
{
	GENERATED_UCLASS_BODY()

public:

	bool bTimerPaused;

	int32 RemainingTime;

	int32 NumTeams;

	TArray<int32> TeamScores;


};
