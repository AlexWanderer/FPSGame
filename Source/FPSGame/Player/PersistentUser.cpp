// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "PersistentUser.h"




UPersistentUser* UPersistentUser::LoadPersistentUser(FString SlotName, const int32 UserIndex)
{
	UPersistentUser* Result = nullptr;

	// first set of player signins can happen before the UWorld exists, which means no OSS, which means no user names, which means no slotnames.
	// Persistent users aren't valid in this state.
	if (SlotName.Len() > 0)
	{
		Result = Cast<UPersistentUser>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
		if (Result == NULL)
		{
			// if failed to load, create a new one
			Result = Cast<UPersistentUser>(UGameplayStatics::CreateSaveGameObject(UPersistentUser::StaticClass()));
		}
		check(Result != NULL);

		Result->SlotName = SlotName;
		Result->UserIndex = UserIndex;
	}

	return Result;
}

void UPersistentUser::TellInputAboutKeybindings()
{

}
