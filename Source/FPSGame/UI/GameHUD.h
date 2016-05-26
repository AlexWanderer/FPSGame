// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/HUD.h"
#include "FPSTypes.h"
#include "GameHUD.generated.h"
/**
 * 
 */
UCLASS()
class FPSGAME_API AGameHUD : public AHUD
{
	GENERATED_BODY()
public:
	/**
	* Toggles in game scoreboard.
	* Note:Will not display if the game menu is visible.

	* @param	bEnable	Required scoreboard display state.
	* @param	bFocus	Give keyboard focus to the scoreboard.
	* @return	true, if the scoreboard visibility changed
	*/
	bool ShowScoreboard(bool bEnable, bool bFocus = false);
	
	/**
	* Set state of current match.
	*
	* @param	NewState	The new match state.
	*/
	void SetMatchState(EShooterMatchState::Type NewState);

	/** Sent from ShooterWeapon, shows NO AMMO text. */
	void NotifyOutOfAmmo();

	/**
	* Sent from pawn hit, used to calculate hit notification overlay for drawing.
	*
	* @param	DamageTaken		The amount of damage.
	* @param	DamageEvent		The actual damage event.
	* @param	PawnInstigator	The pawn that did the damage.
	*/
	void NotifyWeaponHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator);

	/** Notifies we have hit the enemy. */
	void NotifyEnemyHit();
};
