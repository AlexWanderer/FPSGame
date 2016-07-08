// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "Player/FPSPlayerState.h"




int32 AFPSPlayerState::GetTeamNum() const
{
	return TeamNumber;
}

void AFPSPlayerState::AddBulletsFired(int32 NumBullets)
{

}

void AFPSPlayerState::AddRocketsFired(int32 NumRockets)
{

}

bool AFPSPlayerState::IsQuitter() const
{
	return false;
}

void AFPSPlayerState::ScoreKill(AFPSPlayerState* Victim, int32 Points)
{

}

void AFPSPlayerState::ScoreDeath(AFPSPlayerState* KilledBy, int32 Points)
{

}

void AFPSPlayerState::SetTeamNum(int32 NewTeamNumber)
{
	TeamNumber = NewTeamNumber;

	//UpdateTeamColors();
}
