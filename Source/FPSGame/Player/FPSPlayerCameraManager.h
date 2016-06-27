// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Camera/PlayerCameraManager.h"
#include "FPSPlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class AFPSPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

	AFPSPlayerCameraManager();

public:
	
	float NormalFOV;

	float TargetingFOV;

	virtual void UpdateCamera(float DeltaTime) override;
	
};
