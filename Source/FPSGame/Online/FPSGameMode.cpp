// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "FPSGameMode.h"
#include "FPSGameInstance.h"
#include "Player/FPSCharacter.h"
#include "Online/FPSGameState.h"
#include "Online/FPSPlayerState.h"
#include "Player/FPSPlayerController.h"
#include "Bot/FPSBot.h"
#include "Gameplay/TeamStart.h"
#include "Bot/FPSAIController.h"
#include "UI/GameHUD.h"
#include "Online/FPSGameSession.h"

AFPSGameMode::AFPSGameMode() :Super()
{

	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnFinder(TEXT("/Game/Blueprints/Player/PlayerPawn"));
	DefaultPawnClass = PlayerPawnFinder.Class;

	static ConstructorHelpers::FClassFinder<APawn> BotPawnFinder(TEXT("/Game/Blueprints/Bot/Bot"));
	BotPawnClass = BotPawnFinder.Class;

	HUDClass = AGameHUD::StaticClass();

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
		KillerPlayerState->InformAboutKill(KillerPlayerState, DamageType, VictimPlayerState);
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->ScoreDeath(KillerPlayerState, DeathScore);
		VictimPlayerState->BroadcastDeath(KillerPlayerState, DamageType, VictimPlayerState);
	}
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

void AFPSGameMode::PreLogin(const FString& Options, const FString& Address, const TSharedPtr<const FUniqueNetId>& UniqueId, FString& ErrorMessage)
{
	AFPSGameState* const MyGameState = Cast<AFPSGameState>(GameState);
	const bool bMatchIsOver = MyGameState && MyGameState->HasMatchEnded();
	if (bMatchIsOver)
	{
		ErrorMessage = TEXT("Match is over!");
	}
	else
	{
		// GameSession can be NULL if the match is over
		Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	}
}

void AFPSGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// update spectator location for client
	AFPSPlayerController* NewPC = Cast<AFPSPlayerController>(NewPlayer);
	if (NewPC && NewPC->GetPawn() == NULL)
	{
		NewPC->ClientSetSpectatorCamera(NewPC->GetSpawnLocation(), NewPC->GetControlRotation());
	}

	// notify new player if match is already in progress
	if (NewPC && IsMatchInProgress())
	{
		NewPC->ClientGameStarted();
		NewPC->ClientStartOnlineGame();
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

TSubclassOf<AGameSession> AFPSGameMode::GetGameSessionClass() const
{
	return AFPSGameSession::StaticClass();
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
				FinishMatch();

				// Send end round events
				for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
				{
					AFPSPlayerController* PlayerController = Cast<AFPSPlayerController>(*It);

					if (PlayerController && MyGameState)
					{
						AFPSPlayerState* PlayerState = Cast<AFPSPlayerState>((*It)->PlayerState);
						const bool bIsWinner = IsWinner(PlayerState);

						PlayerController->ClientSendRoundEndEvent(bIsWinner, MyGameState->ElapsedTime);
					}
				}
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

void AFPSGameMode::FinishMatch()
{
	AFPSGameState* const MyGameState = Cast<AFPSGameState>(GameState);
	if (IsMatchInProgress())
	{
		EndMatch();
		DetermineMatchWinner();

		// notify players
		for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
		{
			AFPSPlayerState* PlayerState = Cast<AFPSPlayerState>((*It)->PlayerState);
			const bool bIsWinner = IsWinner(PlayerState);

			(*It)->GameHasEnded(NULL, bIsWinner);
		}

		// lock all pawns
		// pawns are not marked as keep for seamless travel, so we will create new pawns on the next match rather than
		// turning these back on.
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
		{
			(*It)->TurnOff();
		}

		// set up to restart the match
		MyGameState->RemainingTime = TimeBetweenMatches;
	}
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
	bNeedsBotCreation = true;
	Super::HandleMatchHasStarted();

	AFPSGameState* const MyGameState = Cast<AFPSGameState>(GameState);
	MyGameState->RemainingTime = RoundTime;
	StartBots();

	// notify players
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		AFPSPlayerController* PC = Cast<AFPSPlayerController>(*It);
		if (PC)
		{
			PC->ClientGameStarted();
		}
	}
}