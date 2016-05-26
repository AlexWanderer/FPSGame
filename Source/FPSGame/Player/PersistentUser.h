// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/SaveGame.h"
#include "PersistentUser.generated.h"

/**
 * 
 */
UCLASS()
class FPSGAME_API UPersistentUser : public USaveGame
{
	GENERATED_BODY()
	
public:

	/** Loads user persistence data if it exists, creates an empty record otherwise. */
	static UPersistentUser* LoadPersistentUser(FString SlotName, const int32 UserIndex);

	/** needed because we can recreate the subsystem that stores it */
	void TellInputAboutKeybindings();
private:
	/** The string identifier used to save/load this persistent user. */
	FString SlotName;
	int32 UserIndex;
};
