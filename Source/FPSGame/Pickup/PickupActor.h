// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "PickupActor.generated.h"

class AFPSCharacter;

UCLASS(Abstract)
class APickupActor : public AActor
{
	GENERATED_BODY()
	
public:	

	APickupActor();

	/** initial setup */
	virtual void BeginPlay() override;

	/** pickup on touch */
	virtual void NotifyActorBeginOverlap(class AActor* Other) override;

	/* The character who has picked up this pickup */
	UPROPERTY(Transient, Replicated)
	AFPSCharacter* PickedUpBy;
private:
	/** FX component */
	UPROPERTY(VisibleDefaultsOnly, Category = Effects)
	UParticleSystemComponent* PickupPSC;

	/************************************************************************/
	/* Active                                                                     */
	/************************************************************************/
	/** FX of active pickup */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* ActiveFX;

	UFUNCTION()
	void OnRep_IsActive();
public:
	/** is it ready for interactions? */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_IsActive)
	uint32 bIsActive : 1;

	/************************************************************************/
	/* Pickup                                                                     */
	/************************************************************************/

	/** sound played when player picks it up */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	USoundCue* PickupSound;

	/** handle touches */
	void PickupOnTouch(class AFPSCharacter* Pawn);

	/** show effects when pickup disappears */
	virtual void OnPickedUp();

public:
	/** blueprint event: pickup disappears */
	UFUNCTION(BlueprintImplementableEvent)
	void OnPickedUpEvent();

	/** check if pawn can use this pickup */
	virtual bool CanBePickedUp(class AFPSCharacter* TestPawn) const;

	/** give pickup */
	virtual void GivePickupTo(class AFPSCharacter* Pawn);
	/************************************************************************/
	/* Respawn                                                                     */
	/************************************************************************/
protected:

	/** how long it takes to respawn? */
	UPROPERTY(EditDefaultsOnly, Category = Pickup)
	float RespawnTime;

	/** sound played on respawn */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	USoundCue* RespawnSound;

	/** FX of pickup on respawn timer */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* RespawningFX;

	/** show and enable pickup */
	virtual void RespawnPickup();

	/** show effects when pickup appears */
	virtual void OnRespawned();

	/** Handle for efficient management of RespawnPickup timer */
	FTimerHandle TimerHandle_RespawnPickup;

public:
	/** blueprint event: pickup appears */
	UFUNCTION(BlueprintImplementableEvent)
	void OnRespawnEvent();

};
