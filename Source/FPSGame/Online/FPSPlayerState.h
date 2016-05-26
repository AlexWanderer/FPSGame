// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerState.h"
#include "FPSPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class AFPSPlayerState : public APlayerState
{
	GENERATED_BODY()

	/** team number */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_TeamColor)
	int32 TeamNumber;

	/** replicate team colors. Updated the players mesh colors appropriately */
	UFUNCTION()
	void OnRep_TeamColor();

public:
	/** get current team */
	int32 GetTeamNum() const;

	//We don't need stats about amount of ammo fired to be server authenticated, so just increment these with local functions
	void AddBulletsFired(int32 NumBullets);
	void AddRocketsFired(int32 NumRockets);



	/** get whether the player quit the match */
	bool IsQuitter() const;
	/** player killed someone */
	void ScoreKill(AFPSPlayerState* Victim, int32 Points);

	/** player died */
	void ScoreDeath(AFPSPlayerState* KilledBy, int32 Points);

	/**
	* Set new team and update pawn. Also updates player character team colors.
	*
	* @param	NewTeamNumber	Team we want to be on.
	*/
	void SetTeamNum(int32 NewTeamNumber);

	/** Sends kill (excluding self) to clients */
	UFUNCTION(Reliable, Client)
	void InformAboutKill(class AFPSPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AFPSPlayerState* KilledPlayerState);

	/** broadcast death to local clients */
	UFUNCTION(Reliable, NetMulticast)
	void BroadcastDeath(class AFPSPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AFPSPlayerState* KilledPlayerState);

};
