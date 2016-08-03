// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Weapon/FPSWeapon.h"
#include "FPSWeaponInstant.generated.h"

USTRUCT()
struct FInstantHitInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector Origin;

	UPROPERTY()
	float ReticleSpread;

	UPROPERTY()
	int32 RandomSeed;
};

USTRUCT()
struct FInstantWeaponData
{
	GENERATED_USTRUCT_BODY()

	/** weapon range */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float WeaponRange;

	/** damage amount */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	int32 HitDamage;

	/** type of damage */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	TSubclassOf<UDamageType> DamageType;

	/** hit verification: scale for bounding box of hit actor */
	UPROPERTY(EditDefaultsOnly, Category = HitVerification)
	float ClientSideHitLeeway;

	/** hit verification: threshold for dot product between view direction and hit direction */
	UPROPERTY(EditDefaultsOnly, Category = HitVerification)
	float AllowedViewDotHitDir;

	FInstantWeaponData()
	{
		WeaponRange = 10000.0f;
		HitDamage = 10;
		DamageType = UDamageType::StaticClass();
		ClientSideHitLeeway = 200.0f;
		AllowedViewDotHitDir = 0.8f;
	}
};

class AFPSImpactEffect;

UCLASS(Abstract)
class AFPSWeaponInstant : public AFPSWeapon
{
	GENERATED_BODY()

	AFPSWeaponInstant();

	/** weapon config */
	UPROPERTY(EditDefaultsOnly, Category = Config)
	FInstantWeaponData InstantConfig;

	/************************************************************************/
	/* Ammo                                                                     */
	/************************************************************************/
	virtual EAmmoType GetAmmoType() const override
	{
		return EAmmoType::EBullet;
	}

	/************************************************************************/
	/* Fire                                                                     */
	/************************************************************************/
private:
	virtual void FireWeapon() override;

	virtual void OnBurstFinished() override;

	/************************************************************************/
	/* Hit&Miss                                                                     */
	/************************************************************************/

	void ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);
	
	void ApplyHit(const FHitResult Impact, FVector ShootDir, int32 RandomSeed, float ReticleSpread);
	
	void ApplyMiss(FVector ShootDir, int32 RandomSeed, float ReticleSpread);
	
	void ProcessInstantHitConfirmed(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);

	/************************************************************************/
	/* HitEffect                                                            */
	/************************************************************************/

	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName TrailTargetParam;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* TrailFX;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
	TSubclassOf<AFPSImpactEffect> ImpactTemplate;

	void SpawnImpactEffects(const FHitResult& Impact);

	void SpawnTrailEffect(const FVector& EndPoint);
	
	/************************************************************************/
	/* Damage                                                            */
	/************************************************************************/

	void DealDamage(const FHitResult& Impact, const FVector& ShootDir);
};
