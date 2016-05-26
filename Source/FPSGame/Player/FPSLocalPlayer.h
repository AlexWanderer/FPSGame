// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/LocalPlayer.h"
#include "FPSLocalPlayer.generated.h"

class UPersistentUser;

UCLASS()
class FPSGAME_API UFPSLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()
public:

	/** Initializes the PersistentUser */
	void LoadPersistentUser();

	class UPersistentUser* GetPersistentUser() const;
private:
	/** Persistent user data stored between sessions (i.e. the user's savegame) */
	UPROPERTY()
	class UPersistentUser* PersistentUser;
};
