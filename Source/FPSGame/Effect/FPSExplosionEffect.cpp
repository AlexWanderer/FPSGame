// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "FPSExplosionEffect.h"


// Sets default values
AFPSExplosionEffect::AFPSExplosionEffect() : Super()
{
	ExplosionLightComponentName = TEXT("ExplosionLight");
	ExplosionLight = CreateDefaultSubobject<UPointLightComponent>(ExplosionLightComponentName);
	RootComponent = ExplosionLight;
	ExplosionLight->AttenuationRadius = 400.0;
	ExplosionLight->Intensity = 500.0f;
	ExplosionLight->bUseInverseSquaredFalloff = false;
	ExplosionLight->LightColor = FColor(255, 185, 35);
	ExplosionLight->CastShadows = false;
	ExplosionLight->bVisible = true;

	PrimaryActorTick.bCanEverTick = true;

	ExplosionLightFadeOut = 0.2f;
}

void AFPSExplosionEffect::BeginPlay()
{
	Super::BeginPlay();

	if (ExplosionFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionFX, GetActorLocation(), GetActorRotation());
	}

	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
	}

	if (Decal.DecalMaterial)
	{
		FRotator RandomDecalRotation = SurfaceHit.ImpactNormal.Rotation();
		RandomDecalRotation.Roll = FMath::FRandRange(-180.0f, 180.0f);

		UGameplayStatics::SpawnDecalAttached(Decal.DecalMaterial, FVector(Decal.DecalSize, Decal.DecalSize, 1.0f),
			SurfaceHit.Component.Get(), SurfaceHit.BoneName,
			SurfaceHit.ImpactPoint, RandomDecalRotation, EAttachLocation::KeepWorldPosition,
			Decal.LifeSpan);
	}
}

void AFPSExplosionEffect::Tick( float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float TimeAlive = GetWorld()->GetTimeSeconds() - CreationTime;
	const float TimeRemaining = FMath::Max(0.0f, ExplosionLightFadeOut - TimeAlive);

	if (TimeRemaining > 0)
	{
		const float FadeAlpha = 1.0f - FMath::Square(TimeRemaining / ExplosionLightFadeOut);

		UPointLightComponent* DefLight = Cast<UPointLightComponent>(GetClass()->GetDefaultSubobjectByName(ExplosionLightComponentName));
		ExplosionLight->SetIntensity(DefLight->Intensity * FadeAlpha);
	}
	else
	{
		Destroy();
	}

}

