// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "FPSTypes.h"
#include "FPSImpactEffect.generated.h"

UCLASS(Abstract, Blueprintable)
class AFPSImpactEffect : public AActor
{
	GENERATED_BODY()
public:
	/************************************************************************/
	/* Effect                                                                 */
	/************************************************************************/
	/** default impact FX used when material specific override doesn't exist */
	UPROPERTY(EditDefaultsOnly, Category = Defaults)
	UParticleSystem* DefaultFX;

	/** impact FX on concrete */
	UPROPERTY(EditDefaultsOnly, Category = Visual)
	UParticleSystem* ConcreteFX;

	/** impact FX on dirt */
	UPROPERTY(EditDefaultsOnly, Category = Visual)
	UParticleSystem* DirtFX;

	/** impact FX on water */
	UPROPERTY(EditDefaultsOnly, Category = Visual)
	UParticleSystem* WaterFX;

	/** impact FX on metal */
	UPROPERTY(EditDefaultsOnly, Category = Visual)
	UParticleSystem* MetalFX;

	/** impact FX on wood */
	UPROPERTY(EditDefaultsOnly, Category = Visual)
	UParticleSystem* WoodFX;

	/** impact FX on glass */
	UPROPERTY(EditDefaultsOnly, Category = Visual)
	UParticleSystem* GlassFX;

	/** impact FX on grass */
	UPROPERTY(EditDefaultsOnly, Category = Visual)
	UParticleSystem* GrassFX;

	/** impact FX on flesh */
	UPROPERTY(EditDefaultsOnly, Category = Visual)
	UParticleSystem* FleshFX;

	/************************************************************************/
	/* Sound                                                                 */
	/************************************************************************/
	/** default impact sound used when material specific override doesn't exist */
	UPROPERTY(EditDefaultsOnly, Category = Defaults)
	USoundCue* DefaultSound;

	/** impact FX on concrete */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* ConcreteSound;

	/** impact FX on dirt */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* DirtSound;

	/** impact FX on water */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* WaterSound;

	/** impact FX on metal */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* MetalSound;

	/** impact FX on wood */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* WoodSound;

	/** impact FX on glass */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* GlassSound;

	/** impact FX on grass */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* GrassSound;

	/** impact FX on flesh */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FleshSound;

	/** default decal when material specific override doesn't exist */
	UPROPERTY(EditDefaultsOnly, Category = Defaults)
	FDecalData DefaultDecal;

	/** surface data for spawning */
	UPROPERTY(BlueprintReadOnly, Category = Surface)
	FHitResult SurfaceHit;

	// Sets default values for this actor's properties
	AFPSImpactEffect();

	/** spawn effect */
	virtual void PostInitializeComponents() override;
	
protected:

	/** get FX for material type */
	UParticleSystem* GetImpactFX(TEnumAsByte<EPhysicalSurface> SurfaceType) const;

	/** get sound for material type */
	USoundCue* GetImpactSound(TEnumAsByte<EPhysicalSurface> SurfaceType) const;
};
