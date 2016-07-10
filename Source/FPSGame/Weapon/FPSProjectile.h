// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Weapon/FPSWeaponProjectile.h"
#include "FPSProjectile.generated.h"

class AFPSExplosionEffect;

UCLASS(Abstract, Blueprintable)
class FPSGAME_API AFPSProjectile : public AActor
{
	GENERATED_BODY()

	AFPSProjectile();

public:

	virtual void PostInitializeComponents() override;

	void InitVelocity(FVector& ShootDirection);

	UFUNCTION()
	void OnImpact(const FHitResult& HitResult);

	FORCEINLINE UProjectileMovementComponent* GetMovementComp() const { return MovementComp; }
	FORCEINLINE USphereComponent* GetCollisionComp() const { return CollisionComp; }
	FORCEINLINE UParticleSystemComponent* GetParticleComp() const { return ParticleComp; }

private:

	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	UProjectileMovementComponent* MovementComp;
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	USphereComponent* CollisionComp;
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	UParticleSystemComponent* ParticleComp;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
	TSubclassOf<AFPSExplosionEffect> ExplosionTemplate;

	TWeakObjectPtr<AController> MyController;

	FProjectileWeaponData WeaponConfig;

	bool bExploded;

	void Explode(const FHitResult& Impact);

	void DisableAndDestroy();
};
