// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "FPSPlayerCameraManager.h"
#include "FPSCharacter.h"

AFPSPlayerCameraManager::AFPSPlayerCameraManager() :Super()
{
	NormalFOV = 90.0f;
	TargetingFOV = 40.f;
	ViewPitchMin = -87.0f;
	ViewPitchMax = 87.0f;
	bAlwaysApplyModifiers = true;
}

void AFPSPlayerCameraManager::UpdateCamera(float DeltaTime)
{
	AFPSCharacter* MyPawn = PCOwner ? Cast<AFPSCharacter>(PCOwner->GetPawn()) : NULL;
	if (MyPawn && MyPawn->IsFirstPerson())
	{
		const float TargetFOV = MyPawn->IsTargeting() ? TargetingFOV : NormalFOV;
		DefaultFOV = FMath::FInterpTo(DefaultFOV, TargetFOV, DeltaTime, 20.0f);
	}

	Super::UpdateCamera(DeltaTime);

	if (MyPawn && MyPawn->IsFirstPerson())
	{
		MyPawn->OnCameraUpdate(GetCameraLocation(), GetCameraRotation());
	}
}
