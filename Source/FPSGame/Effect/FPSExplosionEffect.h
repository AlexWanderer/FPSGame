// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "FPSTypes.h"
#include "FPSExplosionEffect.generated.h"

UCLASS(Abstract, Blueprintable)
class AFPSExplosionEffect : public AActor
{
	GENERATED_BODY()

	AFPSExplosionEffect();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	FName ExplosionLightComponentName;

public:

	UPROPERTY(EditDefaultsOnly, Category = Effect)
	UParticleSystem* ExplosionFX;

	UPROPERTY(EditDefaultsOnly, Category = Effect)
	UPointLightComponent* ExplosionLight;

	UPROPERTY(EditDefaultsOnly, Category = Effect)
	float ExplosionLightFadeOut;

	UPROPERTY(EditDefaultsOnly, Category = Effect)
	USoundCue* ExplosionSound;

	UPROPERTY(EditDefaultsOnly, Category = Effect)
	FDecalData Decal;

	UPROPERTY(BlueprintReadOnly, Category = Surface)
	FHitResult SurfaceHit;

	FORCEINLINE UPointLightComponent* GetExplosionLight() const { return ExplosionLight; }

};
