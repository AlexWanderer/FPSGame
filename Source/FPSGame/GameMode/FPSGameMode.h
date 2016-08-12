// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "FPSGameMode.generated.h"

class AFPSPlayerState;
class AFPSAIController;
class APickupActor;

UCLASS()
class AFPSGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	AFPSGameMode();

	virtual void PreInitializeComponents() override;

	/** Initialize the game. This is called before actors' PreInitializeComponents. */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	/** hides the onscreen hud and restarts the map */
	virtual void RestartGame() override;

	/** select best spawn point for player */
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	/** always pick new random spawn */
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;

	/** always create cheat manager */
	virtual bool AllowCheats(APlayerController* P) override;

	/** returns default pawn class for given controller */
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	//游戏准备开始
	virtual void HandleMatchIsWaitingToStart() override;

	//游戏已经开始
	virtual void HandleMatchHasStarted() override;

	//游戏已经结束
	virtual void HandleMatchHasEnded() override;

	/************************************************************************/
	/* pickup                                                                     */
	/************************************************************************/
public:

	UPROPERTY()
	TArray<APickupActor*> LevelPickups;

	/************************************************************************/
	/* Spawnpoint                                                                */
	/************************************************************************/
protected:

	virtual bool IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const;

	virtual bool IsSpawnpointPreferred(APlayerStart* SpawnPoint, AController* Player) const;

	/************************************************************************/
	/* Match                                                                */
	/************************************************************************/

	UPROPERTY(config)
	int32 TimeBetweenMatches;

	/** delay between first player login and starting match */
	UPROPERTY(config)
	int32 WarmupTime;

	/** match duration */
	UPROPERTY(config)
	int32 RoundTime;

	FTimerHandle TimerHandle_DefaultTimer;
	
	//1. DefaultTimer控制游戏结束
	//2. CheckMatchEnd控制游戏结束
	//3. 命令执行控制游戏结束
	UFUNCTION(exec)
	void FinishMatch(EMatchResult MatchResult);

	bool CheckMatchEnd();

	virtual void DetermineMatchWinner();

	virtual bool IsWinner(AFPSPlayerState* PlayerState) const;

	virtual void DefaultTimer();

	/************************************************************************/
	/* Bot                                                                  */
	/************************************************************************/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameMode)
	TSubclassOf<APawn> BotPawnClass;

	bool bAllowBots;

	UPROPERTY(config)
	int32 MaxBots;

	bool bNeedsBotCreation;

	/** get the name of the bots count option used in server travel URL */
	static FString GetBotsCountOptionName();

	UFUNCTION(exec)
	void SetAllowBots(bool bInAllowBots, int32 InMaxBots = 8);

	/** Creates AIControllers for all bots */
	void CreateBotControllers();

	/** Create a bot */
	AFPSAIController* CreateBot(int32 BotNum);

	/** initialization for bot after creation */
	virtual void InitBot(AFPSAIController* AIController, int32 BotNum);

	/** spawning all bots for this game */
	void StartBots();

	/************************************************************************/
	/* Damage&Die                                                           */
	/************************************************************************/
	/** scale for self instigated damage */
	UPROPERTY(config)
	float DamageSelfScale;

	UPROPERTY(config)
	int32 KillScore;

	UPROPERTY(config)
	int32 DeathScore;

public:

	virtual float ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const;

	virtual bool CanDealDamage(AFPSPlayerState* DamageInstigator, AFPSPlayerState* DamagedPlayer) const;
	
	virtual void Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType);

};
