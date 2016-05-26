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

	/** base weapon spread (degrees) */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float WeaponSpread;

	/** targeting spread modifier */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float TargetingSpreadMod;

	/** continuous firing: spread increment */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float FiringSpreadIncrement;

	/** continuous firing: max increment */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float FiringSpreadMax;

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

	/** defaults */
	FInstantWeaponData()
	{
		WeaponSpread = 5.0f;
		TargetingSpreadMod = 0.25f;
		FiringSpreadIncrement = 1.0f;
		FiringSpreadMax = 10.0f;
		WeaponRange = 10000.0f;
		HitDamage = 10;
		DamageType = UDamageType::StaticClass();
		ClientSideHitLeeway = 200.0f;
		AllowedViewDotHitDir = 0.8f;
	}
};

class AFPSImpactEffect;
// A weapon where the damage impact occurs instantly upon firing
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
	/* Spread                                                                     */
	/************************************************************************/

	/** current spread from continuous firing */
	float CurrentFiringSpread;

	/** get current spread */
	float GetCurrentSpread() const;

	/************************************************************************/
	/* Fire                                                                     */
	/************************************************************************/
	/** [local] weapon specific fire implementation */
	virtual void FireWeapon() override;
	/** [local + server] update spread on firing */
	virtual void OnBurstFinished() override;

	/************************************************************************/
	/* Hit&Miss                                                                     */
	/************************************************************************/
	/** process the instant hit and notify the server if necessary */
	void ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);
	
	/** server notified of hit from client to verify */
	UFUNCTION(reliable, server, WithValidation)
	void ServerNotifyHit(const FHitResult Impact, FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread);
	
	/** server notified of miss to show trail FX */
	UFUNCTION(unreliable, server, WithValidation)
	void ServerNotifyMiss(FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread);
	
	/** continue processing the instant hit, as if it has been confirmed by the server */
	void ProcessInstantHitConfirmed(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);

	/** instant hit notify for replication */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_HitNotify)
	FInstantHitInfo HitNotify;

	UFUNCTION()
	void OnRep_HitNotify();

	/************************************************************************/
	/* HitEffect                                                            */
	/************************************************************************/
	/** param name for beam target in smoke trail */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName TrailTargetParam;

	/** smoke trail */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* TrailFX;

	/** impact effects */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	TSubclassOf<AFPSImpactEffect> ImpactTemplate;

	/** called in network play to do the cosmetic fx  */
	void SimulateInstantHit(const FVector& ShotOrigin, int32 RandomSeed, float ReticleSpread);

	/** spawn effects for impact */
	void SpawnImpactEffects(const FHitResult& Impact);

	/** spawn trail effect */
	void SpawnTrailEffect(const FVector& EndPoint);
	
	/************************************************************************/
	/* Damage                                                            */
	/************************************************************************/
	/** check if weapon should deal damage to actor */
	bool ShouldDealDamage(AActor* TestActor) const;

	/** handle damage */
	void DealDamage(const FHitResult& Impact, const FVector& ShootDir);
};
