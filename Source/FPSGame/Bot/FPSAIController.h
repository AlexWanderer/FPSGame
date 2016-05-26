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

	/** Update direction AI is looking based on FocalPoint */
	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn = true) override;

public:
	void CheckAmmo(const class AFPSWeapon* CurrentWeapon);

	/************************************************************************/
	/* BehaviorTree                                                                     */
	/************************************************************************/
private:
	int32 EnemyKeyID;
	int32 NeedAmmoKeyID;

	UPROPERTY(transient)
	UBlackboardComponent* BlackboardComp;

	/* Cached BT component */
	UPROPERTY(transient)
	UBehaviorTreeComponent* BehaviorComp;
public:
	/** Returns BlackboardComp subobject **/
	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; }
	/** Returns BehaviorComp subobject **/
	FORCEINLINE UBehaviorTreeComponent* GetBehaviorComp() const { return BehaviorComp; }
	
	/************************************************************************/
	/*     Respawn                                                     */
	/************************************************************************/
protected:
	/** Handle for efficient management of Respawn timer */
	FTimerHandle TimerHandle_Respawn;
public:

	void Respawn();

	/************************************************************************/
	/*    Behavior                                                     */
	/************************************************************************/
public:

	void SetEnemy(class APawn* InPawn);
	class AFPSCharacter* GetEnemy() const;

	/* If there is line of sight to current enemy, start firing at them */
	UFUNCTION(BlueprintCallable, Category = Behavior)
	void ShootEnemy();

	/* Finds the closest enemy and sets them as current target */
	UFUNCTION(BlueprintCallable, Category = Behavior)
	void FindClosestEnemy();

	UFUNCTION(BlueprintCallable, Category = Behavior)
	bool FindClosestEnemyWithLOS(AFPSCharacter* ExcludeEnemy);

	bool HasWeaponLOSToEnemy(AActor* InEnemyActor, const bool bAnyEnemy) const;
};
