// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "PickupAmmo.h"
#include "Weapon/FPSWeapon.h"

APickupAmmo::APickupAmmo() : Super()
{
	AmmoClips = 2;
}


bool APickupAmmo::CanBePickedUp(AFPSCharacter* TestPawn) const
{
	AFPSWeapon* TestWeapon = (TestPawn ? TestPawn->FindWeapon(WeaponType) : NULL);
	if (bIsActive && TestWeapon)
	{
		return TestWeapon->GetCurrentAmmo() < TestWeapon->GetMaxAmmo();
	}

	return false;
}

bool APickupAmmo::IsForWeapon(UClass* WeaponClass)
{
	return WeaponType->IsChildOf(WeaponClass);
}

void APickupAmmo::GivePickupTo(AFPSCharacter* Pawn)
{
	AFPSWeapon* Weapon = (Pawn ? Pawn->FindWeapon(WeaponType) : NULL);
	if (Weapon)
	{
		int32 Qty = AmmoClips * Weapon->GetAmmoPerClip();
		Weapon->GiveAmmo(Qty);

	}
}
