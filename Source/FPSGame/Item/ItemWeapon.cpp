// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "ItemWeapon.h"
#include "Player/FPSCharacter.h"
#include "Weapon/FPSWeapon.h"


AItemWeapon::AItemWeapon() : Super()
{

}

void AItemWeapon::OnUsed(APawn* UserPawn)
{
	AFPSCharacter* ThePawn = Cast<AFPSCharacter>(UserPawn);

	if (ThePawn)
	{
		FActorSpawnParameters SpawnParam;
		SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AFPSWeapon* NewWeapon = GetWorld()->SpawnActor<AFPSWeapon>(WeaponClass, SpawnParam);

		ThePawn->AddWeapon(NewWeapon);
		ThePawn->EquipWeapon(NewWeapon);
		Super::OnUsed(UserPawn);
	}
}
