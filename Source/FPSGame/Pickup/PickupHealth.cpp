// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "PickupHealth.h"

APickupHealth::APickupHealth() : Super()
{
	Health = 50;
}



bool APickupHealth::CanBePickedUp(AFPSCharacter* TestPawn) const
{
	return TestPawn && (TestPawn->Health < TestPawn->GetMaxHealth());
}

void APickupHealth::GivePickupTo(AFPSCharacter* Pawn)
{
	if (Pawn)
	{
		Pawn->Health = FMath::Min(FMath::TruncToInt(Pawn->Health) + Health, Pawn->GetMaxHealth());
	}
}
