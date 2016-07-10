// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "FPSWeaponProjectile.h"
#include "FPSProjectile.h"

AFPSWeaponProjectile::AFPSWeaponProjectile() : Super()
{

}

void AFPSWeaponProjectile::ApplyWeaponConfig(FProjectileWeaponData& Data)
{
	Data = ProjectileConfig;
}

void AFPSWeaponProjectile::FireWeapon()
{
	FVector ShootDir = GetAdjustedAimDirection();
	FVector Origin = GetMuzzleLocation();

	const float ProjecileAdjustRange = 10000.f;
	const FVector StartTrace = GetShootStartLocation(ShootDir);
	const FVector EndTrace = StartTrace + ShootDir * ProjecileAdjustRange;
	FHitResult Impact = WeaponTrace(StartTrace, EndTrace);

	//½ÃÕý·½Ïò
	if (Impact.bBlockingHit)
	{
		const FVector AdjustedShootDir = (Impact.ImpactPoint - Origin).GetSafeNormal();

		bool bWeaponPenetration = false;

		const float DirectionDot = FVector::DotProduct(AdjustedShootDir, ShootDir);

		if (DirectionDot < 0.f)
		{
			// shooting backwards = weapon is penetrating
			bWeaponPenetration = true;
		}
		else if (DirectionDot < 0.5f)
		{
			// check for weapon penetration if angle difference is big enough
			// raycast along weapon mesh to check if there's blocking hit

			FVector MuzzleStartTrace = Origin - GetMuzzleDirection() * 150.0f;
			FVector MuzzleEndTrace = Origin;
			FHitResult MuzzleImpact = WeaponTrace(MuzzleStartTrace, MuzzleEndTrace);

			if (MuzzleImpact.bBlockingHit)
			{
				bWeaponPenetration = true;
			}
		}

		if (bWeaponPenetration)
		{
			// spawn at crosshair position
			Origin = Impact.ImpactPoint - ShootDir * 10.0f;
		}
		else
		{
			// adjust direction to hit
			ShootDir = AdjustedShootDir;
		}
	}

	SpawnProjectile(Origin, ShootDir);
}

void AFPSWeaponProjectile::SpawnProjectile(FVector Origin, FVector ShootDir)
{
	FTransform SpawnTM(ShootDir.Rotation(), Origin);
	AFPSProjectile* Projectile = Cast<AFPSProjectile>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ProjectileConfig.ProjectileClass, SpawnTM));
	if (Projectile)
	{
		Projectile->Instigator = Instigator;
		Projectile->SetOwner(this);
		UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);

		Projectile->InitVelocity(ShootDir);
	}
}
