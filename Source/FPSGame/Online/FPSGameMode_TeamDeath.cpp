// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "FPSGameMode_TeamDeath.h"
#include "Online/FPSPlayerState.h"
#include "Online/FPSGameState.h"
#include "Bot/FPSAIController.h"
#include "Gameplay/TeamStart.h"

AFPSGameMode_TeamDeath::AFPSGameMode_TeamDeath(): Super()
{
	NumTeams = 2;
	//bDelayedStart = true;
}

void AFPSGameMode_TeamDeath::PostLogin(APlayerController* NewPlayer)
{
	// Place player on a team before Super (VoIP team based init, findplayerstart, etc)
	AFPSPlayerState* NewPlayerState = CastChecked<AFPSPlayerState>(NewPlayer->PlayerState);
	const int32 TeamNum = ChooseTeam(NewPlayerState);
	NewPlayerState->SetTeamNum(TeamNum);

	Super::PostLogin(NewPlayer);
}

void AFPSGameMode_TeamDeath::InitGameState()
{
	Super::InitGameState();

	AFPSGameState* const MyGameState = Cast<AFPSGameState>(GameState);
	if (MyGameState)
	{
		MyGameState->NumTeams = NumTeams;
	}
}

bool AFPSGameMode_TeamDeath::CanDealDamage(AFPSPlayerState* DamageInstigator, AFPSPlayerState* DamagedPlayer) const
{
	return DamageInstigator && DamagedPlayer && (DamagedPlayer == DamageInstigator || DamagedPlayer->GetTeamNum() != DamageInstigator->GetTeamNum());

}

int32 AFPSGameMode_TeamDeath::ChooseTeam(AFPSPlayerState* ForPlayerState) const
{
	TArray<int32> TeamBalance;
	TeamBalance.AddZeroed(NumTeams);

	// get current team balance
	for (int32 i = 0; i < GameState->PlayerArray.Num(); i++)
	{
		AFPSPlayerState const* const TestPlayerState = Cast<AFPSPlayerState>(GameState->PlayerArray[i]);
		if (TestPlayerState && TestPlayerState != ForPlayerState && TeamBalance.IsValidIndex(TestPlayerState->GetTeamNum()))
		{
			TeamBalance[TestPlayerState->GetTeamNum()]++;
		}
	}

	// find least populated one
	int32 BestTeamScore = TeamBalance[0];
	for (int32 i = 1; i < TeamBalance.Num(); i++)
	{
		if (BestTeamScore > TeamBalance[i])
		{
			BestTeamScore = TeamBalance[i];
		}
	}

	// there could be more than one...
	TArray<int32> BestTeams;
	for (int32 i = 0; i < TeamBalance.Num(); i++)
	{
		if (TeamBalance[i] == BestTeamScore)
		{
			BestTeams.Add(i);
		}
	}

	// get random from best list
	const int32 RandomBestTeam = BestTeams[FMath::RandHelper(BestTeams.Num())];
	return RandomBestTeam;

}

void AFPSGameMode_TeamDeath::DetermineMatchWinner()
{
	AFPSGameState const* const MyGameState = Cast<AFPSGameState>(GameState);
	int32 BestScore = MAX_uint32;
	int32 BestTeam = -1;
	int32 NumBestTeams = 1;

	for (int32 i = 0; i < MyGameState->TeamScores.Num(); i++)
	{
		const int32 TeamScore = MyGameState->TeamScores[i];
		if (BestScore < TeamScore)
		{
			BestScore = TeamScore;
			BestTeam = i;
			NumBestTeams = 1;
		}
		else if (BestScore == TeamScore)
		{
			NumBestTeams++;
		}
	}

	WinnerTeam = (NumBestTeams == 1) ? BestTeam : NumTeams;
}

bool AFPSGameMode_TeamDeath::IsWinner(AFPSPlayerState* PlayerState) const
{
	return PlayerState && !PlayerState->IsQuitter() && PlayerState->GetTeamNum() == WinnerTeam;
}

void AFPSGameMode_TeamDeath::InitBot(AFPSAIController* AIC, int32 BotNum)
{
	AFPSPlayerState* BotPlayerState = CastChecked<AFPSPlayerState>(AIC->PlayerState);
	const int32 TeamNum = ChooseTeam(BotPlayerState);
	BotPlayerState->SetTeamNum(TeamNum);

	Super::InitBot(AIC, BotNum);
}

bool AFPSGameMode_TeamDeath::IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const
{
	if (Player)
	{
		ATeamStart* TeamStart = Cast<ATeamStart>(SpawnPoint);
		AFPSPlayerState* PlayerState = Cast<AFPSPlayerState>(Player->PlayerState);

		if (PlayerState && TeamStart && TeamStart->SpawnTeam != PlayerState->GetTeamNum())
		{
			return false;
		}
	}

	return Super::IsSpawnpointAllowed(SpawnPoint, Player);
}
