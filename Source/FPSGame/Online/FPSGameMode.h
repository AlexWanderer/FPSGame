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

	/** Accept or reject a player attempting to join the server.  Fails login if you set the ErrorMessage to a non-empty string. */
	virtual void PreLogin(const FString& Options, const FString& Address, const TSharedPtr<const FUniqueNetId>& UniqueId, FString& ErrorMessage) override;

	/** starts match warmup */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/** hides the onscreen hud and restarts the map */
	virtual void RestartGame() override;

	/** Returns game session class to use */
	virtual TSubclassOf<AGameSession> GetGameSessionClass() const override;

	/** select best spawn point for player */
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	/** always pick new random spawn */
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;

	/** always create cheat manager */
	virtual bool AllowCheats(APlayerController* P) override;

	/** returns default pawn class for given controller */
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	/** Handle for efficient management of DefaultTimer timer */
	FTimerHandle TimerHandle_DefaultTimer;

	/** update remaining time */
	virtual void DefaultTimer();
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
	/** check if player can use spawnpoint */
	virtual bool IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const;

	/** check if player should use spawnpoint */
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
	
	/** finish current match and lock players */
	UFUNCTION(exec)
	void FinishMatch();

	/** check who won */
	virtual void DetermineMatchWinner();

	/** check if PlayerState is a winner */
	virtual bool IsWinner(AFPSPlayerState* PlayerState) const;

	/** called before startmatch */
	virtual void HandleMatchIsWaitingToStart() override;

	/** starts new match */
	virtual void HandleMatchHasStarted() override;

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

	/** score for kill */
	UPROPERTY(config)
	int32 KillScore;

	/** score for death */
	UPROPERTY(config)
	int32 DeathScore;
public:
	/** prevents friendly fire */
	virtual float ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const;
	
	/** can players damage each other? */
	virtual bool CanDealDamage(AFPSPlayerState* DamageInstigator, AFPSPlayerState* DamagedPlayer) const;
	
	/** notify about kills */
	virtual void Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType);

};
