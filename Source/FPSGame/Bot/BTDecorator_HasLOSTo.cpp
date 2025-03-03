// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "BTDecorator_HasLOSTo.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "Bot/FPSAIController.h"
#include "Bot/FPSBot.h"
#include "Player/FPSPlayerState.h"

UBTDecorator_HasLOSTo::UBTDecorator_HasLOSTo() : Super()
{
	NodeName = "Has LOS To";
	// accept only actors and vectors	
	EnemyKey.AddObjectFilter(this, *NodeName, AActor::StaticClass());
	EnemyKey.AddVectorFilter(this, *NodeName);
}

bool UBTDecorator_HasLOSTo::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
	AAIController* MyController = OwnerComp.GetAIOwner();
	bool HasLOS = false;

	if (MyController && MyBlackboard)
	{
		auto MyID = MyBlackboard->GetKeyID(EnemyKey.SelectedKeyName);
		auto TargetKeyType = MyBlackboard->GetKeyType(MyID);

		FVector TargetLocation;
		bool bGotTarget = false;
		AActor* EnemyActor = NULL;
		if (TargetKeyType == UBlackboardKeyType_Object::StaticClass())
		{
			UObject* KeyValue = MyBlackboard->GetValue<UBlackboardKeyType_Object>(MyID);
			EnemyActor = Cast<AActor>(KeyValue);
			if (EnemyActor)
			{
				TargetLocation = EnemyActor->GetActorLocation();
				bGotTarget = true;
			}
		}
		else if (TargetKeyType == UBlackboardKeyType_Vector::StaticClass())
		{
			TargetLocation = MyBlackboard->GetValue<UBlackboardKeyType_Vector>(MyID);
			bGotTarget = true;
		}

		if (bGotTarget == true)
		{
			if (LOSTrace(OwnerComp.GetOwner(), EnemyActor, TargetLocation) == true)
			{
				HasLOS = true;
			}
		}
	}

	return HasLOS;
}

bool UBTDecorator_HasLOSTo::LOSTrace(AActor* InActor, AActor* InEnemyActor, const FVector& EndLocation) const
{
	static FName LosTag = FName(TEXT("AILosTrace"));
	AFPSAIController* MyController = Cast<AFPSAIController>(InActor);
	AFPSBot* MyBot = MyController ? Cast<AFPSBot>(MyController->GetPawn()) : NULL;

	bool bHasLOS = false;

	{
		if (MyBot != NULL)
		{
			// Perform trace to retrieve hit info
			FCollisionQueryParams TraceParams(LosTag, true, InActor);
			TraceParams.bTraceAsyncScene = true;

			TraceParams.bReturnPhysicalMaterial = true;
			TraceParams.AddIgnoredActor(MyBot);
			const FVector StartLocation = MyBot->GetActorLocation();
			FHitResult Hit(ForceInit);
			GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, COLLISION_WEAPON, TraceParams);
			if (Hit.bBlockingHit == true)
			{
				// We hit something. If we have an actor supplied, just check if the hit actor is an enemy. If it is consider that 'has LOS'
				AActor* HitActor = Hit.GetActor();
				if (Hit.GetActor() != NULL)
				{
					// If the hit is our target actor consider it LOS
					if (HitActor == InActor)
					{
						bHasLOS = true;
					}
					else
					{
						// Check the team of us against the team of the actor we hit if we are able. If they dont match good to go.
						ACharacter* HitChar = Cast<ACharacter>(HitActor);
						if ((HitChar != NULL)
							&& (MyController->PlayerState != NULL) && (HitChar->PlayerState != NULL))
						{
							AFPSPlayerState* HitPlayerState = Cast<AFPSPlayerState>(HitChar->PlayerState);
							AFPSPlayerState* MyPlayerState = Cast<AFPSPlayerState>(MyController->PlayerState);
							if ((HitPlayerState != NULL) && (MyPlayerState != NULL))
							{
								if (HitPlayerState->GetTeamNum() != MyPlayerState->GetTeamNum())
								{
									bHasLOS = true;
								}
							}
						}
					}
				}
				else //we didnt hit an actor
				{
					if (InEnemyActor == NULL)
					{
						// We were not given an actor - so check of the distance between what we hit and the target. If what we hit is further away than the target we should be able to hit our target.
						FVector HitDelta = Hit.ImpactPoint - StartLocation;
						FVector TargetDelta = EndLocation - StartLocation;
						if (TargetDelta.Size() < HitDelta.Size())
						{
							bHasLOS = true;
						}
					}
				}
			}
		}
	}

	return bHasLOS;
}


/*
bool UBTDecorator_HasLoSTo::CalculateRawConditionValue(class UBehaviorTreeComponent* OwnerComp, uint8* NodeMemory) const
{
const UBlackboardComponent* BlackboardComp = OwnerComp->GetBlackboardComponent();
if (BlackboardComp == NULL)
{
return false;
}

FVector PointA = FVector::ZeroVector;
FVector PointB = FVector::ZeroVector;
const bool bHasPointA = BlackboardComp->GetLocationFromEntry(BlackboardKeyA.GetSelectedKeyID(), PointA);
const bool bHasPointB = BlackboardComp->GetLocationFromEntry(BlackboardKeyB.GetSelectedKeyID(), PointB);

bool bHasPath = false;

const UNavigationSystem* NavSys = UNavigationSystem::GetCurrent(OwnerComp->GetWorld());
if (NavSys && bHasPointA && bHasPointB)
{
const AAIController* AIOwner = Cast<AAIController>(OwnerComp->GetOwner());
const ANavigationData* NavData = AIOwner && AIOwner->NavComponent ? AIOwner->NavComponent->GetNavData() : NULL;
TSharedPtr<const FNavigationQueryFilter> QueryFilter = UNavigationQueryFilter::GetQueryFilter(NavData, FilterClass);

if (PathQueryType == EPathExistanceQueryType::NavmeshRaycast2D)
{
#if WITH_RECAST
const ARecastNavMesh* RecastNavMesh = Cast<const ARecastNavMesh>(NavData);
bHasPath = RecastNavMesh && RecastNavMesh->IsSegmentOnNavmesh(PointA, PointB, QueryFilter);
#endif
}
else
{
EPathFindingMode::Type TestMode = (PathQueryType == EPathExistanceQueryType::HierarchicalQuery) ? EPathFindingMode::Hierarchical : EPathFindingMode::Regular;
bHasPath = NavSys->TestPathSync(FPathFindingQuery(AIOwner, NavData, PointA, PointB, QueryFilter), TestMode);
}
}

return bHasPath;
}*/