// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "FPSPlayerController.generated.h"

class AGameHUD;
class UPersistentUser;

UCLASS()
class FPSGAME_API AFPSPlayerController : public APlayerController
{
	GENERATED_BODY()

	AFPSPlayerController();

	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;

	virtual void FailedToSpawnPawn() override;

	virtual void PawnPendingDestroy(APawn* Pawn) override;

	virtual void SetCinematicMode(bool bInCinematicMode, bool bHidePlayer, bool bAffectsHUD, bool bAffectsMovement, bool bAffectsTurning) override;

	virtual bool IsMoveInputIgnored() const override;

	virtual bool IsLookInputIgnored() const override;

	virtual void GameHasEnded(class AActor* EndGameFocus /* = NULL */, bool bIsWinner /* = false */) override;

	/************************************************************************/
	/* input							                                     */
	/************************************************************************/
public:
	uint8 bAllowGameActions : 1;

	bool IsGameInputAllowed() const;


	/************************************************************************/
	/*  Death						                                     */
	/************************************************************************/

	FVector LastDeathLocation;

	bool FindDeathCameraSpot(FVector& CameraLocation, FRotator& CameraRotation);

	/************************************************************************/
	/*  HUD							                                     */
	/************************************************************************/
public:
	AGameHUD* GetGameHUD() const;

	/************************************************************************/
	/*  Cheat                                       */
	/************************************************************************/
public:

	bool HasInfiniteClip() const;

	bool HasInfiniteAmmo() const;

	bool HasHealthRegen() const;

	bool HasGodMode() const;

};
