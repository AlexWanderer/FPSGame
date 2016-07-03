// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "FPSWeaponInstant.h"
#include "Effect/FPSImpactEffect.h"

AFPSWeaponInstant::AFPSWeaponInstant():Super()
{
	CurrentFiringSpread = 0.0f;
}


float AFPSWeaponInstant::GetCurrentSpread() const
{
	float FinalSpread = InstantConfig.WeaponSpread + CurrentFiringSpread;
	if (MyPawn && MyPawn->IsTargeting())
	{
		FinalSpread *= InstantConfig.TargetingSpreadMod;
	}

	return FinalSpread;
}

void AFPSWeaponInstant::FireWeapon()
{
	const int32 RandomSeed = FMath::Rand();
	FRandomStream WeaponRandomStream(RandomSeed);
	const float CurrentSpread = GetCurrentSpread();
	const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * 0.5f);

	const FVector AimDir = GetAdjustedAim();
	const FVector StartTrace = GetCameraDamageStartLocation(AimDir);
	const FVector ShootDir = WeaponRandomStream.VRandCone(AimDir, ConeHalfAngle, ConeHalfAngle);
	const FVector EndTrace = StartTrace + ShootDir * InstantConfig.WeaponRange;

	const FHitResult Impact = WeaponTrace(StartTrace, EndTrace);
	ProcessInstantHit(Impact, StartTrace, ShootDir, RandomSeed, CurrentSpread);

	CurrentFiringSpread = FMath::Min(InstantConfig.FiringSpreadMax, CurrentFiringSpread + InstantConfig.FiringSpreadIncrement);

}

void AFPSWeaponInstant::OnBurstFinished()
{
	Super::OnBurstFinished();

	CurrentFiringSpread = 0.0f;
}

void AFPSWeaponInstant::ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread)
{
	if (MyPawn)
	{
		if (Impact.GetActor())
		{
			NotifyHit(Impact, ShootDir, RandomSeed, ReticleSpread);
		}
		else if (Impact.GetActor() == NULL)
		{
			if (Impact.bBlockingHit)
			{
				NotifyHit(Impact, ShootDir, RandomSeed, ReticleSpread);
			}
			else
			{
				NotifyMiss(ShootDir, RandomSeed, ReticleSpread);
			}
		}
	}
}

void AFPSWeaponInstant::ProcessInstantHitConfirmed(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread)
{
	//击中Actor了，才有伤害
	if (Impact.GetActor())
	{
		DealDamage(Impact, ShootDir);
	}

	const FVector EndTrace = Origin + ShootDir * InstantConfig.WeaponRange;
	const FVector EndPoint = Impact.GetActor() ? Impact.ImpactPoint : EndTrace;

	SpawnTrailEffect(EndPoint);
	SpawnImpactEffects(Impact);
}

void AFPSWeaponInstant::SpawnImpactEffects(const FHitResult& Impact)
{
	if (ImpactTemplate && Impact.bBlockingHit)
	{
		FHitResult UseImpact = Impact;

		// trace again to find component lost during replication
		if (!Impact.Component.IsValid())
		{
			const FVector StartTrace = Impact.ImpactPoint + Impact.ImpactNormal * 10.0f;
			const FVector EndTrace = Impact.ImpactPoint - Impact.ImpactNormal * 10.0f;
			FHitResult Hit = WeaponTrace(StartTrace, EndTrace);
			UseImpact = Hit;
		}

		FTransform const SpawnTransform(Impact.ImpactNormal.Rotation(), Impact.ImpactPoint);
		AFPSImpactEffect* EffectActor = GetWorld()->SpawnActorDeferred<AFPSImpactEffect>(ImpactTemplate, SpawnTransform);
		if (EffectActor)
		{
			EffectActor->SurfaceHit = UseImpact;
			UGameplayStatics::FinishSpawningActor(EffectActor, SpawnTransform);
		}
	}
}

void AFPSWeaponInstant::SpawnTrailEffect(const FVector& EndPoint)
{
	if (TrailFX)
	{
		const FVector Origin = GetMuzzleLocation();

		UParticleSystemComponent* TrailPSC = UGameplayStatics::SpawnEmitterAtLocation(this, TrailFX, Origin);
		if (TrailPSC)
		{
			TrailPSC->SetVectorParameter(TrailTargetParam, EndPoint);
		}
	}
}


void AFPSWeaponInstant::DealDamage(const FHitResult& Impact, const FVector& ShootDir)
{
	FPointDamageEvent PointDmg;
	PointDmg.DamageTypeClass = InstantConfig.DamageType;
	PointDmg.HitInfo = Impact;
	PointDmg.ShotDirection = ShootDir;
	PointDmg.Damage = InstantConfig.HitDamage;

	Impact.GetActor()->TakeDamage(PointDmg.Damage, PointDmg, MyPawn->Controller, this);
}

void AFPSWeaponInstant::NotifyHit(const FHitResult Impact, FVector ShootDir, int32 RandomSeed, float ReticleSpread)
{
	const float WeaponAngleDot = FMath::Abs(FMath::Sin(ReticleSpread * PI / 180.f));

	if (Instigator && (Impact.GetActor() || Impact.bBlockingHit))
	{
		const FVector Origin = GetMuzzleLocation();
		const FVector ViewDir = (Impact.Location - Origin).GetSafeNormal();

		const float ViewDotHitDir = FVector::DotProduct(Instigator->GetViewRotation().Vector(), ViewDir);
		if (ViewDotHitDir > InstantConfig.AllowedViewDotHitDir - WeaponAngleDot)
		{
			if (CurrentState != EWeaponState::Idle)
			{
				if (Impact.GetActor() == NULL)
				{
					if (Impact.bBlockingHit)
					{
						ProcessInstantHitConfirmed(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
					}
				}
				//处理静态Actor
				else if (Impact.GetActor()->IsRootComponentStatic() || Impact.GetActor()->IsRootComponentStationary())
				{
					ProcessInstantHitConfirmed(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
				}
				//处理动态Actor
				else
				{
					// Get the component bounding box
					const FBox HitBox = Impact.GetActor()->GetComponentsBoundingBox();

					// calculate the box extent, and increase by a leeway
					FVector BoxExtent = 0.5 * (HitBox.Max - HitBox.Min);
					BoxExtent *= InstantConfig.ClientSideHitLeeway;

					// avoid precision errors with really thin objects
					BoxExtent.X = FMath::Max(20.0f, BoxExtent.X);
					BoxExtent.Y = FMath::Max(20.0f, BoxExtent.Y);
					BoxExtent.Z = FMath::Max(20.0f, BoxExtent.Z);

					// Get the box center
					const FVector BoxCenter = (HitBox.Min + HitBox.Max) * 0.5;

					// if we are within client tolerance
					if (FMath::Abs(Impact.Location.Z - BoxCenter.Z) < BoxExtent.Z &&
						FMath::Abs(Impact.Location.X - BoxCenter.X) < BoxExtent.X &&
						FMath::Abs(Impact.Location.Y - BoxCenter.Y) < BoxExtent.Y)
					{
						ProcessInstantHitConfirmed(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
					}
					else
					{
						UE_LOG(LogTemp, Log, TEXT("%s Rejected hit of %s (outside bounding box tolerance)"), *GetNameSafe(this), *GetNameSafe(Impact.GetActor()));
					}
				}
			}
		}
		else if (ViewDotHitDir <= InstantConfig.AllowedViewDotHitDir)
		{
			UE_LOG(LogTemp, Log, TEXT("%s Rejected hit of %s (facing too far from the hit direction)"), *GetNameSafe(this), *GetNameSafe(Impact.GetActor()));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("%s Rejected hit of %s"), *GetNameSafe(this), *GetNameSafe(Impact.GetActor()));
		}
	}
}


void AFPSWeaponInstant::NotifyMiss(FVector ShootDir, int32 RandomSeed, float ReticleSpread)
{
	const FVector Origin = GetMuzzleLocation();

	const FVector EndTrace = Origin + ShootDir * InstantConfig.WeaponRange;
	SpawnTrailEffect(EndTrace);
}
