// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "GameHUD.h"




void AGameHUD::SetMatchState(EShooterMatchState NewState)
{
	CurrentMatchState = NewState;
	ReceiveMatchStateChanged(CurrentMatchState);
}

bool AGameHUD::ShowScoreboard(bool bEnable, bool bFocus /*= false*/)
{
	return false;
}

void AGameHUD::NotifyOutOfAmmo()
{

}

void AGameHUD::NotifyWeaponHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator)
{

}

void AGameHUD::NotifyEnemyHit()
{

}
