// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameMode/FPSGameMode.h"
#include "FPSGameMode_TeamDeath.generated.h"

class AFPSPlayerState;
class AFPSPlayerController;
class AFPSAIController;

UCLASS()
class AFPSGameMode_TeamDeath : public AFPSGameMode
{
	GENERATED_BODY()

	AFPSGameMode_TeamDeath();
	
	/** setup team changes at player login */
	void PostLogin(APlayerController* NewPlayer) override;

	/** initialize replicated game data */
	virtual void InitGameState() override;

	/** can players damage each other? */
	virtual bool CanDealDamage(AFPSPlayerState* DamageInstigator, AFPSPlayerState* DamagedPlayer) const override;

protected:

	/** number of teams */
	int32 NumTeams;

	/** best team */
	int32 WinnerTeam;

	/** pick team with least players in or random when it's equal */
	int32 ChooseTeam(AFPSPlayerState* ForPlayerState) const;

	/** check who won */
	virtual void DetermineMatchWinner() override;

	/** check if PlayerState is a winner */
	virtual bool IsWinner(AFPSPlayerState* PlayerState) const override;

	/** initialization for bot after spawning */
	virtual void InitBot(AFPSAIController* AIC, int32 BotNum) override;


	/** check team constraints */
	virtual bool IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const override;
};
