// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Weapon/FPSWeapon.h"
#include "FPSWeaponProjectile.generated.h"

class AFPSProjectile;

USTRUCT()
struct FProjectileWeaponData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = ProjectileWeaponData)
	TSubclassOf<AFPSProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = ProjectileWeaponData)
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, Category = ProjectileWeaponData)
	float ProjectileLife;

	UPROPERTY(EditDefaultsOnly, Category = ProjectileWeaponData)
	int32 ExplosionDamage;

	UPROPERTY(EditDefaultsOnly, Category = ProjectileWeaponData)
	float ExplosionRadius;

	FProjectileWeaponData()
	{
		ProjectileClass = NULL;
		ProjectileLife = 10.0f;
		ExplosionDamage = 100;
		ExplosionRadius = 300.0f;
		DamageType = UDamageType::StaticClass();
	}
};

UCLASS(Abstract)
class AFPSWeaponProjectile : public AFPSWeapon
{
	GENERATED_BODY()

	AFPSWeaponProjectile();
public:

	void ApplyWeaponConfig(FProjectileWeaponData& Data);

protected:

	virtual void FireWeapon() override;

	void SpawnProjectile(FVector Origin, FVector ShootDir);

	FORCEINLINE virtual EAmmoType GetAmmoType() const override { return EAmmoType::ERocket; }

	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
	FProjectileWeaponData ProjectileConfig;
};
