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

	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

	/** true for the first frame after the game has ended */
	uint8 bGameEndedFrame : 1;

	/************************************************************************/
	/*  PersistentUser					                                     */
	/************************************************************************/
	/** Returns the persistent user record associated with this player, or null if there is't one. */
	class UPersistentUser* GetPersistentUser() const;

	/************************************************************************/
	/*  Gameplay Actions					                                     */
	/************************************************************************/
public:
	/** if set, gameplay related actions (movement, weapn usage, etc) are allowed */
	uint8 bAllowGameActions : 1;

	/** check if gameplay related actions (movement, weapon usage, etc) are allowed right now */
	bool IsGameInputAllowed() const;


	/************************************************************************/
	/*  DeathCamera						                                     */
	/************************************************************************/

	/** stores pawn location at last player death, used where player scores a kill after they died **/
	FVector LastDeathLocation;

	/** try to find spot for death cam */
	bool FindDeathCameraSpot(FVector& CameraLocation, FRotator& CameraRotation);

	/************************************************************************/
	/*  HUD & UI						                                     */
	/************************************************************************/
	/** Returns a pointer to the shooter game hud. May return NULL. */
	AGameHUD* GetGameHUD() const;

	/************************************************************************/
	/*  APlayerController interface                                         */
	/************************************************************************/
public:
	/** handle weapon visibility */
	virtual void SetCinematicMode(bool bInCinematicMode, bool bHidePlayer, bool bAffectsHUD, bool bAffectsMovement, bool bAffectsTurning) override;

	/** Returns true if movement input is ignored. Overridden to always allow spectators to move. */
	virtual bool IsMoveInputIgnored() const override;

	/** Returns true if look input is ignored. Overridden to always allow spectators to look around. */
	virtual bool IsLookInputIgnored() const override;

	/** initialize the input system from the player settings */
	virtual void InitInputSystem() override;

	/************************************************************************/
	/*  AController interface                                         */
	/************************************************************************/
protected:
	/** transition to dead state, retries spawning later */
	virtual void FailedToSpawnPawn() override;

	/** update camera when pawn dies */
	virtual void PawnPendingDestroy(APawn* Pawn) override;

	/************************************************************************/
	/*  Online                                        */
	/************************************************************************/

public:
	/** Notifies clients to send the end-of-round event */
	UFUNCTION(reliable, client)
	void ClientSendRoundEndEvent(bool bIsWinner, int32 ExpendedTimeInSeconds);
	
	/** sets spectator location and rotation */
	UFUNCTION(reliable, client)
	void ClientSetSpectatorCamera(FVector CameraLocation, FRotator CameraRotation);
	
	/** notify player about started match */
	UFUNCTION(reliable, client)
	void ClientGameStarted();

	/** Starts the online game using the session name in the PlayerState */
	UFUNCTION(reliable, client)
	void ClientStartOnlineGame();

	/** Handle for efficient management of ClientStartOnlineGame timer */
	FTimerHandle TimerHandle_ClientStartOnlineGame;


	/************************************************************************/
	/*  Cheat                                       */
	/************************************************************************/
	/** get infinite clip cheat */
	bool HasInfiniteClip() const;

	/** get infinite ammo cheat */
	bool HasInfiniteAmmo() const;

	/** get health regen cheat */
	bool HasHealthRegen() const;

	/** get gode mode cheat */
	bool HasGodMode() const;

};
