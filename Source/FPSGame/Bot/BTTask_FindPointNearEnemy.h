// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_FindPointNearEnemy.generated.h"


// Bot AI task that tries to find a location near the current enemy
UCLASS()
class UBTTask_FindPointNearEnemy : public UBTTask_BlackboardBase
{
	GENERATED_BODY()


	UBTTask_FindPointNearEnemy();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
