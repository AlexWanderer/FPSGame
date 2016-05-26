// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "BTTask_FindPickup.h"
#include "Bot/FPSAIController.h"
#include "Bot/FPSBot.h"
#include "Online/FPSGameMode.h"
#include "Pickup/PickupAmmo.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "Weapon/FPSWeaponInstant.h"

UBTTask_FindPickup::UBTTask_FindPickup():Super()
{
}

EBTNodeResult::Type UBTTask_FindPickup::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AFPSAIController* MyController = Cast<AFPSAIController>(OwnerComp.GetAIOwner());
	AFPSBot* MyBot = MyController ? Cast<AFPSBot>(MyController->GetPawn()) : NULL;
	if (MyBot == NULL)
	{
		return EBTNodeResult::Failed;
	}

	AFPSGameMode* GameMode = MyBot->GetWorld()->GetAuthGameMode<AFPSGameMode>();
	if (GameMode == NULL)
	{
		return EBTNodeResult::Failed;
	}

	const FVector MyLoc = MyBot->GetActorLocation();
	APickupAmmo* BestPickup = NULL;
	float BestDistSq = MAX_FLT;

	for (int32 i = 0; i < GameMode->LevelPickups.Num(); ++i)
	{
		APickupAmmo* AmmoPickup = Cast<APickupAmmo>(GameMode->LevelPickups[i]);
		if (AmmoPickup && AmmoPickup->IsForWeapon(AFPSWeaponInstant::StaticClass()) && AmmoPickup->CanBePickedUp(MyBot))
		{
			const float DistSq = (AmmoPickup->GetActorLocation() - MyLoc).SizeSquared();
			if (BestDistSq == -1 || DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestPickup = AmmoPickup;
			}
		}
	}

	if (BestPickup)
	{
		OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID(), BestPickup->GetActorLocation());
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
