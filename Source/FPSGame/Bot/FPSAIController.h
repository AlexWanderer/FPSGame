// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "FPSAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class AFPSWeapon;

UCLASS()
class AFPSAIController : public AAIController
{
	GENERATED_BODY()

	AFPSAIController();

	virtual void GameHasEnded(class AActor* EndGameFocus = NULL, bool bIsWinner = false) override;
	virtual void Possess(class APawn* InPawn) override;
	virtual void BeginInactiveState() override;
	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn = true) override;

public:
	void CheckAmmo(const class AFPSWeapon* CurrentWeapon);

	/************************************************************************/
	/* BehaviorTree                                                                     */
	/************************************************************************/
private:

	//Blackboard Data
	int32 EnemyKeyID;
	int32 NeedAmmoKeyID;

	UBlackboardComponent* BlackboardComp;
	UBehaviorTreeComponent* BehaviorComp;

public:

	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; }
	FORCEINLINE UBehaviorTreeComponent* GetBehaviorComp() const { return BehaviorComp; }
	
	/************************************************************************/
	/*     Respawn                                                     */
	/************************************************************************/
public:
	FTimerHandle TimerHandle_Respawn;
	void Respawn();

	/************************************************************************/
	/*    Behavior                                                     */
	/************************************************************************/
public:

	void SetEnemy(class APawn* InPawn);
	AFPSCharacter* GetEnemy() const;

	UFUNCTION(BlueprintCallable, Category = FPSAIController)
	void ShootEnemy();

	//寻找最近的敌人
	UFUNCTION(BlueprintCallable, Category = FPSAIController)
	void FindClosestEnemy();

	//寻找最近的敌人（视线检测）
	UFUNCTION(BlueprintCallable, Category = FPSAIController)
	bool FindClosestEnemyWithLOS(AFPSCharacter* ExcludeEnemy);

	bool HasWeaponLOSToEnemy(AActor* InEnemyActor, const bool bAnyEnemy) const;
};
