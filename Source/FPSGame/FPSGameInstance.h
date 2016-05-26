// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"
#include "FPSGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class FPSGAME_API UFPSGameInstance : public UGameInstance
{
	GENERATED_BODY()

	UFPSGameInstance();
	/************************************************************************/
	/*   Online                                                             */
	/************************************************************************/
private:
	/** Whether the match is online or not */
	bool bIsOnline;
public:
	/** Returns true if the game is in online mode */
	bool GetIsOnline() const { return bIsOnline; }
	/** Sets the online mode of the game */
	void SetIsOnline(bool bInIsOnline);
};
