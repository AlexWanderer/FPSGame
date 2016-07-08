// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerState.h"
#include "FPSPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class AFPSPlayerState : public APlayerState
{
	GENERATED_BODY()


	int32 TeamNumber;

public:

	int32 GetTeamNum() const;

	void AddBulletsFired(int32 NumBullets);
	void AddRocketsFired(int32 NumRockets);

	bool IsQuitter() const;

	void ScoreKill(AFPSPlayerState* Victim, int32 Points);

	void ScoreDeath(AFPSPlayerState* KilledBy, int32 Points);

	void SetTeamNum(int32 NewTeamNumber);
};
