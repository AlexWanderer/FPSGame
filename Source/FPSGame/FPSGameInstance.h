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
	bool bIsOnline;
public:
	bool GetIsOnline() const { return bIsOnline; }
	void SetIsOnline(bool bInIsOnline);
};
