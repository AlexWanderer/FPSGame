// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Pickup/PickupActor.h"
#include "PickupAmmo.generated.h"

class AFPSCharacter;
class AFPSWeapon;

UCLASS(Abstract)
class APickupAmmo : public APickupActor
{
	GENERATED_BODY()

	APickupAmmo();
public:
	/** check if pawn can use this pickup */
	virtual bool CanBePickedUp(AFPSCharacter* TestPawn) const override;

	bool IsForWeapon(UClass* WeaponClass);

protected:

	/** how much ammo does it give? */
	UPROPERTY(EditDefaultsOnly, Category = Pickup)
	int32 AmmoClips;

	/** which weapon gets ammo? */
	UPROPERTY(EditDefaultsOnly, Category = Pickup)
	TSubclassOf<AFPSWeapon> WeaponType;

	/** give pickup */
	virtual void GivePickupTo(AFPSCharacter* Pawn) override;
};
