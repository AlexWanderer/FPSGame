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
	/** normal FOV */
	float NormalFOV;

	/** targeting FOV */
	float TargetingFOV;

	/** After updating camera, inform pawn to update 1p mesh to match camera's location&rotation */
	virtual void UpdateCamera(float DeltaTime) override;
	
	
};