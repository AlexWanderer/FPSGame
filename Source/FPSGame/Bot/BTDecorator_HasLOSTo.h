// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_HasLOSTo.generated.h"

// Checks if the AI pawn has Line of sight to the specified Actor or Location(Vector).
UCLASS()
class UBTDecorator_HasLOSTo : public UBTDecorator
{
	GENERATED_BODY()

	UBTDecorator_HasLOSTo();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

protected:

	UPROPERTY(EditAnywhere, Category = Condition)
	struct FBlackboardKeySelector EnemyKey;

private:
	bool LOSTrace(AActor* InActor, AActor* InEnemyActor, const FVector& EndLocation) const;
	
	
	
};
