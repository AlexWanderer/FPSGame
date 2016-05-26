// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "FPSLocalPlayer.h"
#include "Player/PersistentUser.h"

void UFPSLocalPlayer::LoadPersistentUser()
{

}

class UPersistentUser* UFPSLocalPlayer::GetPersistentUser() const
{
	// if persistent data isn't loaded yet, load it
	if (PersistentUser == nullptr)
	{
		UFPSLocalPlayer* const MutableThis = const_cast<UFPSLocalPlayer*>(this);
		// casting away constness to enable caching implementation behavior
		MutableThis->LoadPersistentUser();
	}
	return PersistentUser;
}
