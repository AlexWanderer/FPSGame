// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Item/ItemActor.h"
#include "ItemWeapon.generated.h"

class AFPSWeapon;

UCLASS()
class FPSGAME_API AItemWeapon : public AItemActor
{
	GENERATED_BODY()
	
public:

	AItemWeapon();

	UPROPERTY(EditDefaultsOnly, Category = "ItemWeapon")
	TSubclassOf<AFPSWeapon> WeaponClass;

	virtual void OnUsed(APawn* UserPawn) override;
	
};
