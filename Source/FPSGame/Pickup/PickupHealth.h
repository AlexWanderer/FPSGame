// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Pickup/PickupActor.h"
#include "PickupHealth.generated.h"

class AFPSCharacter;

UCLASS(Abstract)
class APickupHealth : public APickupActor
{
	GENERATED_BODY()


	APickupHealth();

public:	
	
	/** check if pawn can use this pickup */
	virtual bool CanBePickedUp(AFPSCharacter* TestPawn) const override;

protected:

	/** how much health does it give? */
	UPROPERTY(EditDefaultsOnly, Category = Pickup)
	int32 Health;

	/** give pickup */
	virtual void GivePickupTo(AFPSCharacter* Pawn) override;
};
