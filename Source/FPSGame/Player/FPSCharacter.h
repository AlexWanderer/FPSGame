// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "FPSCharacter.generated.h"

/*
Category:

-PROPERTY
1. Sound
2. Animation
3. Effect
4. CharacterConfig
5. Inventory
*/
class AFPSWeapon;
class AUsableActor;
UCLASS(Abstract)
class AFPSCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()

public:

	virtual void BeginPlay() override;

	virtual void Tick( float DeltaSeconds ) override;

	virtual void Destroyed() override;

	virtual void FellOutOfWorld(const UDamageType& dmgType) override;

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	virtual float PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None) override;

	virtual void StopAnimMontage(class UAnimMontage* AnimMontage) override;


	void OnCameraUpdate(const FVector& CameraLocation, const FRotator& CameraRotation);

	UFUNCTION(BlueprintCallable, Category = FPSCharacter)
	FRotator GetAimOffsets() const;

	bool IsEnemyFor(AController* TestPC) const;

	/************************************************************************/
	/* Input                                                                */
	/************************************************************************/
public:

	void OnMoveForward(float Val);

	void OnMoveRight(float Val);

	void OnMoveUp(float Val);

	void OnStartFire();

	void OnStopFire();

	void OnStartTargeting();

	void OnStopTargeting();

	void OnNextWeapon();

	void OnPrevWeapon();

	void OnReload();

	void OnStartJump();

	void OnStopJump();

	void OnStartRunning();

	void OnStopRunning();

	void OnUse();

	void OnDrop();

	/************************************************************************/
	/* Respawn                                                              */
	/************************************************************************/
protected:

	UPROPERTY(EditDefaultsOnly, Category = Effect)
	UParticleSystem* RespawnFX;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* RespawnSound;

	/************************************************************************/
	/* Damage & death                                                     */
	/************************************************************************/
public:

	//自杀
	virtual void Suicide();

	//他杀
	virtual void KillBy(APawn* EventInstigator);

	//Check当前是否可以死亡
	virtual bool CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const;

protected:

	virtual void Hit(float DamageTaken, struct FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser);
	virtual bool Die(float KillingDamage, struct FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser);
	virtual void DieActual(float KillingDamage, struct FDamageEvent const& DamageEvent, APawn* InstigatingPawn, AActor* DamageCauser);

	void EnableRagdollPhysics();

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* DeathAnim;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* DeathSound;

	UPROPERTY(BlueprintReadOnly, Category = FPSCharacter)
	uint32 bIsDying : 1;

	/************************************************************************/
	/* Healt                                                                 */
	/************************************************************************/
public:

	//默认最大生命值
	UFUNCTION(BlueprintCallable, Category = FPSCharacter)
	int32 GetMaxHealth() const;

	bool IsAlive() const;

	// 当前生命值
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CharacterConfig)
	float Health;

private:

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* LowHealthSound;

	float LowHealthPercentage;

	UPROPERTY()
	UAudioComponent* LowHealthWarningPlayer;

	/************************************************************************/
	/* Inventory                                                            */
	/************************************************************************/
public:

	FName GetWeaponAttachPoint() const;

	//取当前装备的武器
	UFUNCTION(BlueprintCallable, Category = FPSCharacter)
	AFPSWeapon* GetWeapon() const;

	//从背包中找一把武器
	AFPSWeapon* FindWeapon(TSubclassOf<AFPSWeapon> WeaponClass);

	//添加武器进背包
	void AddWeapon(AFPSWeapon* Weapon);

	//删除武器从背包
	void RemoveWeapon(AFPSWeapon* Weapon);

	//装备武器
	void EquipWeapon(AFPSWeapon* Weapon);

	//取消装备武器
	void UnEquipWeapon(AFPSWeapon* Weapon);

protected:

	void SpawnDefaultInventory();
	void DestroyInventory();
	void SetCurrentWeapon(AFPSWeapon* NewWeapon, AFPSWeapon* LastWeapon = NULL);

	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TArray<TSubclassOf<AFPSWeapon> > DefaultInventoryClasses;

	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	FName WeaponAttachPoint;

	TArray<AFPSWeapon*> Inventory;

	AFPSWeapon* WeaponCurrent;

	float DropWeaponMaxDistance = 100;

	/************************************************************************/
	/* Run                                                         */
	/************************************************************************/

public:

	//Check冲刺
	UFUNCTION(BlueprintCallable, Category = FPSCharacter)
	bool IsRunning() const;

	//取冲刺时的速度加成系数
	UFUNCTION(BlueprintCallable, Category = FPSCharacter)
	float GetRunningSpeedModifier() const;

protected:

	//冲刺状态
	uint8 bWantsToRun : 1;

	//触发式的冲刺，触发后可以一直维持冲刺
	uint8 bWantsToRunToggled : 1;

	void SetRunning(bool bNewRunning, bool bToggle);
	void UpdateRunSounds(bool bNewRunning);

	UPROPERTY()
	UAudioComponent* RunLoopAC;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* RunLoopSound;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* RunStopSound;

	UPROPERTY(EditDefaultsOnly, Category = CharacterConfig)
	float RunningSpeedModifier;

	/************************************************************************/
	/* Fire                                                         */
	/************************************************************************/
public:

	bool CanFire() const;

	void StartWeaponFire();

	void StopWeaponFire();

protected:

	uint8 bWantsToFire : 1;

	/************************************************************************/
	/* Reload                                                            */
	/************************************************************************/
public:

	bool CanReload() const;

	/************************************************************************/
	/* Targeting                                                         */
	/************************************************************************/
public:

	UFUNCTION(BlueprintCallable, Category = FPSCharacter)
	bool IsTargeting() const;

	UFUNCTION(BlueprintCallable, Category = FPSCharacter)
	float GetTargetingSpeedModifier() const;

protected:

	UPROPERTY(EditDefaultsOnly, Category = CharacterConfig)
	float TargetingSpeedModifier;

	uint8 bIsTargeting : 1;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* TargetingSound;

	void SetTargeting(bool bNewTargeting);

	/************************************************************************/
	/* Animation                                                         */
	/************************************************************************/
public:

	void StopAllAnimMontages();

	/************************************************************************/
	/* Interactive                                                    */
	/************************************************************************/
private:

	bool bHasNewFocus = true;
	float MaxUseDistance = 500;
	AUsableActor* FocusedUsableActor;

	AUsableActor* GetUsableActorInView();
	void TickViewUsable();

	/************************************************************************/
	/* Mesh                                                           */
	/************************************************************************/
public:

	UFUNCTION(BlueprintCallable, Category = FPSCharacter)
	virtual bool IsFirstPerson() const;

	USkeletalMeshComponent* GetPawnMesh() const;

	USkeletalMeshComponent* GetSpecifcPawnMesh(bool bWantFirstPerson) const;

protected:

	void UpdatePawnMeshes();

private:

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh1P;

	/************************************************************************/
	/* TeamColor                                                            */
	/************************************************************************/

	/** material instances for setting team color in mesh (3rd person view) */
	TArray<UMaterialInstanceDynamic*> MeshMIDs;

	void UpdateTeamColors(UMaterialInstanceDynamic* UseMID);

	void UpdateTeamColorsAllMIDs();
};
