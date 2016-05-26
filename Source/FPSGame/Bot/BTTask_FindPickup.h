// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_FindPickup.generated.h"


// Bot AI Task that attempts to locate a pickup 
UCLASS()
class UBTTask_FindPickup : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

	UBTTask_FindPickup();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
