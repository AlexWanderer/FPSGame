// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "FPSGameMode.h"
#include "FPSGameInstance.h"
#include "Player/FPSCharacter.h"
#include "GameMode/FPSGameState.h"
#include "Player/FPSPlayerState.h"
#include "Player/FPSPlayerController.h"
#include "Bot/FPSBot.h"
#include "Gameplay/TeamStart.h"
#include "Bot/FPSAIController.h"
#include "UI/GameHUD.h"

AFPSGameMode::AFPSGameMode() :Super()
{

	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnFinder(TEXT("/Game/Blueprints/Player/Player_Pawn"));
	DefaultPawnClass = PlayerPawnFinder.Class;

	static ConstructorHelpers::FClassFinder<APawn> BotPawnFinder(TEXT("/Game/Blueprints/Bot/Bot_Shooter"));
	BotPawnClass = BotPawnFinder.Class;

	static ConstructorHelpers::FClassFinder<AHUD> HUDFinder(TEXT("/Game/Blueprints/HUD/InGameHUD"));
	HUDClass = HUDFinder.Class;

	PlayerControllerClass = AFPSPlayerController::StaticClass();
	PlayerStateClass = AFPSPlayerState::StaticClass();
	GameStateClass = AFPSGameState::StaticClass();

	MinRespawnDelay = 5.0f;

	bAllowBots = true;
	bNeedsBotCreation = true;
	bUseSeamlessTravel = true;
}

void AFPSGameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	GetWorldTimerManager().SetTimer(TimerHandle_DefaultTimer, this, &AFPSGameMode::DefaultTimer, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}

FString AFPSGameMode::GetBotsCountOptionName()
{
	return FString(TEXT("Bots"));
}

void AFPSGameMode::SetAllowBots(bool bInAllowBots, int32 InMaxBots /*= 8*/)
{
	bAllowBots = bInAllowBots;
	MaxBots = InMaxBots;
}

void AFPSGameMode::CreateBotControllers()
{
	UWorld* World = GetWorld();
	int32 ExistingBots = 0;
	for (FConstControllerIterator It = World->GetControllerIterator(); It; ++It)
	{
		AFPSAIController* AIC = Cast<AFPSAIController>(*It);
		if (AIC)
		{
			++ExistingBots;
		}
	}

	// Create any necessary AIControllers.  Hold off on Pawn creation until pawns are actually necessary or need recreating.	
	int32 BotNum = ExistingBots;
	for (int32 i = 0; i < MaxBots - ExistingBots; ++i)
	{
		CreateBot(BotNum + i);
	}
}

AFPSAIController* AFPSGameMode::CreateBot(int32 BotNum)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = nullptr;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.OverrideLevel = nullptr;

	UWorld* World = GetWorld();
	AFPSAIController* AIC = World->SpawnActor<AFPSAIController>(SpawnInfo);
	InitBot(AIC, BotNum);

	return AIC;
}

void AFPSGameMode::InitBot(AFPSAIController* AIController, int32 BotNum)
{
	if (AIController)
	{
		if (AIController->PlayerState)
		{
			FString BotName = FString::Printf(TEXT("Bot %d"), BotNum);
			AIController->PlayerState->PlayerName = BotName;
		}
	}
}

void AFPSGameMode::StartBots()
{
	// checking number of existing human player.
	int32 NumPlayers = 0;
	int32 NumBots = 0;
	UWorld* World = GetWorld();
	for (FConstControllerIterator It = World->GetControllerIterator(); It; ++It)
	{
		AFPSAIController* AIC = Cast<AFPSAIController>(*It);
		if (AIC)
		{
			RestartPlayer(AIC);
		}
	}
}

float AFPSGameMode::ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	float ActualDamage = Damage;

	AFPSCharacter* DamagedPawn = Cast<AFPSCharacter>(DamagedActor);
	if (DamagedPawn && EventInstigator)
	{
		AFPSPlayerState* DamagedPlayerState = Cast<AFPSPlayerState>(DamagedPawn->PlayerState);
		AFPSPlayerState* InstigatorPlayerState = Cast<AFPSPlayerState>(EventInstigator->PlayerState);

		// disable friendly fire
		if (!CanDealDamage(InstigatorPlayerState, DamagedPlayerState))
		{
			ActualDamage = 0.0f;
		}

		// scale self instigated damage
		if (InstigatorPlayerState == DamagedPlayerState)
		{
			ActualDamage *= DamageSelfScale;
		}
	}

	return ActualDamage;
}

bool AFPSGameMode::CanDealDamage(AFPSPlayerState* DamageInstigator, AFPSPlayerState* DamagedPlayer) const
{
	return true;
}

void AFPSGameMode::Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	AFPSPlayerState* KillerPlayerState = Killer ? Cast<AFPSPlayerState>(Killer->PlayerState) : NULL;
	AFPSPlayerState* VictimPlayerState = KilledPlayer ? Cast<AFPSPlayerState>(KilledPlayer->PlayerState) : NULL;

	if (KillerPlayerState && KillerPlayerState != VictimPlayerState)
	{
		KillerPlayerState->ScoreKill(VictimPlayerState, KillScore);
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->ScoreDeath(KillerPlayerState, DeathScore);
	}

	CheckMatchEnd();
}

void AFPSGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	const int32 BotsCountOptionValue = UGameplayStatics::GetIntOption(Options, GetBotsCountOptionName(), 0);
	SetAllowBots(BotsCountOptionValue > 0 ? true : false, BotsCountOptionValue);
	Super::InitGame(MapName, Options, ErrorMessage);

	const UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance && Cast<UFPSGameInstance>(GameInstance)->GetIsOnline())
	{
		bPauseable = false;
	}
}

void AFPSGameMode::RestartGame()
{
	// Hide the scoreboard too !
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		AFPSPlayerController* PlayerController = Cast<AFPSPlayerController>(*It);
		if (PlayerController != nullptr)
		{
			AGameHUD* ShooterHUD = Cast<AGameHUD>(PlayerController->GetHUD());
			if (ShooterHUD != nullptr)
			{
				// Passing true to bFocus here ensures that focus is returned to the game viewport.
				ShooterHUD->ShowScoreboard(false, true);
			}
		}
	}

	Super::RestartGame();
}

AActor* AFPSGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> PreferredSpawns;
	TArray<APlayerStart*> FallbackSpawns;

	APlayerStart* BestStart = NULL;
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* TestSpawn = *It;
		if (TestSpawn->IsA<APlayerStartPIE>())
		{
			// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
			BestStart = TestSpawn;
			break;
		}
		else
		{
			if (IsSpawnpointAllowed(TestSpawn, Player))
			{
				if (IsSpawnpointPreferred(TestSpawn, Player))
				{
					PreferredSpawns.Add(TestSpawn);
				}
				else
				{
					FallbackSpawns.Add(TestSpawn);
				}
			}
		}
	}


	if (BestStart == NULL)
	{
		if (PreferredSpawns.Num() > 0)
		{
			BestStart = PreferredSpawns[FMath::RandHelper(PreferredSpawns.Num())];
		}
		else if (FallbackSpawns.Num() > 0)
		{
			BestStart = FallbackSpawns[FMath::RandHelper(FallbackSpawns.Num())];
		}
	}

	return BestStart ? BestStart : Super::ChoosePlayerStart_Implementation(Player);

}

bool AFPSGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	return false;
}

bool AFPSGameMode::AllowCheats(APlayerController* P)
{
	return true;
}

UClass* AFPSGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (InController->IsA<AFPSAIController>())
	{
		return BotPawnClass;
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void AFPSGameMode::DefaultTimer()
{
	// don't update timers for Play In Editor mode, it's not real match
	if (GetWorld()->IsPlayInEditor())
	{
		// start match if necessary.
		if (GetMatchState() == MatchState::WaitingToStart)
		{
			StartMatch();
		}
		return;
	}

	//根据剩余时间决定游戏情况
	AFPSGameState* const MyGameState = Cast<AFPSGameState>(GameState);
	if (MyGameState && MyGameState->RemainingTime > 0 && !MyGameState->bTimerPaused)
	{
		MyGameState->RemainingTime--;

		if (MyGameState->RemainingTime <= 0)
		{
			if (GetMatchState() == MatchState::WaitingPostMatch)
			{
				RestartGame();
			}
			else if (GetMatchState() == MatchState::InProgress)
			{
				FinishMatch(EMatchResult::Timeout);
			}
			else if (GetMatchState() == MatchState::WaitingToStart)
			{
				StartMatch();
			}
		}
	}
}

bool AFPSGameMode::IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const
{
	ATeamStart* ShooterSpawnPoint = Cast<ATeamStart>(SpawnPoint);
	if (ShooterSpawnPoint)
	{
		AFPSAIController* AIController = Cast<AFPSAIController>(Player);
		if (ShooterSpawnPoint->bNotForBots && AIController)
		{
			return false;
		}

		if (ShooterSpawnPoint->bNotForPlayers && AIController == NULL)
		{
			return false;
		}
		return true;
	}

	return false;
}

bool AFPSGameMode::IsSpawnpointPreferred(APlayerStart* SpawnPoint, AController* Player) const
{
	ACharacter* MyPawn = Cast<ACharacter>((*DefaultPawnClass)->GetDefaultObject<ACharacter>());
	AFPSAIController* AIController = Cast<AFPSAIController>(Player);
	if (AIController != nullptr)
	{
		MyPawn = Cast<ACharacter>(BotPawnClass->GetDefaultObject<ACharacter>());
	}

	if (MyPawn)
	{
		const FVector SpawnLocation = SpawnPoint->GetActorLocation();
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
		{
			ACharacter* OtherPawn = Cast<ACharacter>(*It);
			if (OtherPawn && OtherPawn != MyPawn)
			{
				const float CombinedHeight = (MyPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + OtherPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) * 2.0f;
				const float CombinedRadius = MyPawn->GetCapsuleComponent()->GetScaledCapsuleRadius() + OtherPawn->GetCapsuleComponent()->GetScaledCapsuleRadius();
				const FVector OtherLocation = OtherPawn->GetActorLocation();

				// check if player start overlaps this pawn
				if (FMath::Abs(SpawnLocation.Z - OtherLocation.Z) < CombinedHeight && (SpawnLocation - OtherLocation).Size2D() < CombinedRadius)
				{
					return false;
				}
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}

void AFPSGameMode::FinishMatch(EMatchResult MatchResult)
{
	AFPSGameState* const MyGameState = Cast<AFPSGameState>(GameState);
	if (IsMatchInProgress())
	{
		EndMatch();
		DetermineMatchWinner();

		APlayerController* TheController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (MatchResult == EMatchResult::Win)
		{
			TheController->GameHasEnded(nullptr, true);
		}
		else if (MatchResult == EMatchResult::Lost)
		{
			TheController->GameHasEnded(nullptr, false);
		}
		else if (MatchResult == EMatchResult::Timeout)
		{
			TheController->GameHasEnded(nullptr, false);
		}
		else
		{
			TheController->GameHasEnded(nullptr, false);
		}

		// lock all pawns
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
		{
			(*It)->TurnOff();
		}

		// set up to restart the match
		MyGameState->RemainingTime = TimeBetweenMatches;
	}
}

bool AFPSGameMode::CheckMatchEnd()
{
	bool bHasAlivePlayer = false;
	bool bHasAliveBot = false;

	//检查是否还有玩家存活
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; It++)
	{
		AFPSCharacter* MyPawn = Cast<AFPSCharacter>(*It);
		if (MyPawn && MyPawn->IsAlive())
		{
			AFPSPlayerState* PS = Cast<AFPSPlayerState>(MyPawn->PlayerState);
			if (PS)
			{
				if (!PS->bIsABot)
				{
					/* Found one player that is still alive, game will continue */
					bHasAlivePlayer = true;
					break;
				}
			}
		}
	}

	//检查是否还有Bot存活
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; It++)
	{
		AFPSBot* Bot = Cast<AFPSBot>(*It);
		if (Bot && Bot->IsAlive())
		{
			bHasAliveBot = true;
			break;
		}
	}

	if (bHasAlivePlayer && !bHasAliveBot)
	{
		FinishMatch(EMatchResult::Win);
		return true;
	}
	else if (!bHasAlivePlayer)
	{
		FinishMatch(EMatchResult::Lost);
		return true;
	}

	return false;
}

void AFPSGameMode::DetermineMatchWinner()
{

}

bool AFPSGameMode::IsWinner(AFPSPlayerState* PlayerState) const
{
	return false;
}

void AFPSGameMode::HandleMatchIsWaitingToStart()
{
	if (bNeedsBotCreation)
	{
		CreateBotControllers();
		bNeedsBotCreation = false;
	}

	if (bDelayedStart)
	{
		// start warmup if needed
		AFPSGameState* const MyGameState = Cast<AFPSGameState>(GameState);
		if (MyGameState && MyGameState->RemainingTime == 0)
		{
			const bool bWantsMatchWarmup = !GetWorld()->IsPlayInEditor();
			if (bWantsMatchWarmup && WarmupTime > 0)
			{
				MyGameState->RemainingTime = WarmupTime;
			}
			else
			{
				MyGameState->RemainingTime = 0.0f;
			}
		}
	}
}

void AFPSGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	bNeedsBotCreation = true;
	AFPSGameState* const MyGameState = Cast<AFPSGameState>(GameState);
	MyGameState->RemainingTime = RoundTime;
	StartBots();
}

void AFPSGameMode::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
	
}
