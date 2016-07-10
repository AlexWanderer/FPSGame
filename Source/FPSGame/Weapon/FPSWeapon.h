// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "FPSWeapon.generated.h"


namespace EWeaponState
{
	enum Type
	{
		Idle,
		Firing,
		Reloading,
		Equipping,
	};
}

enum class EAmmoType
{
	EBullet,
	ERocket,
	EMax,
};

USTRUCT()
struct FInstigatorAnim
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* Pawn1P;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* Pawn3P;
};

USTRUCT()
struct FWeaponData
{
	GENERATED_USTRUCT_BODY()

	//是否无限弹药
	UPROPERTY(EditDefaultsOnly, Category = WeaponData)
	bool bInfiniteAmmo;

	//是否无限弹夹
	UPROPERTY(EditDefaultsOnly, Category = WeaponData)
	bool bInfiniteClip;

	//最大弹药容量
	UPROPERTY(EditDefaultsOnly, Category = WeaponData)
	int32 MaxAmmo;

	//每弹夹内的子弹数
	UPROPERTY(EditDefaultsOnly, Category = WeaponData)
	int32 AmmoPerClip;

	//初始弹夹数
	UPROPERTY(EditDefaultsOnly, Category = WeaponData)
	int32 InitialClips;

	//开火状态下的每次射击间隔时间
	UPROPERTY(EditDefaultsOnly, Category = WeaponData)
	float TimeBetweenShots;

	//没有装弹动画情况下需要的装弹完成时间
	UPROPERTY(EditDefaultsOnly, Category = WeaponData)
	float NoAnimReloadDuration;

	FWeaponData()
	{
		bInfiniteAmmo = false;
		bInfiniteClip = false;
		MaxAmmo = 100;
		AmmoPerClip = 20;
		InitialClips = 4;
		TimeBetweenShots = 0.2f;
		NoAnimReloadDuration = 1.0f;
	}
};

/*
Category:

-PROPERTY
1. Sound
2. Animation
3. Effect
4. WeaponConfig
*/

class AItemWeapon;
UCLASS()
class AFPSWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AFPSWeapon();

	/** perform initial setup */
	virtual void PostInitializeComponents() override;

	virtual void Destroyed() override;

	/************************************************************************/
	/* Config                                                            */
	/************************************************************************/
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
	FWeaponData WeaponConfig;

	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
	TSubclassOf<AItemWeapon> ItemClass;

	/************************************************************************/
	/* Calculate                                                            */
	/************************************************************************/

	FVector GetCameraAim() const;

	//取当前角色瞄准的方向
	virtual FVector GetAdjustedAimDirection() const;

	//取当前射击的起始位置
	FVector GetShootStartLocation(const FVector& AimDir) const;

	//取枪口的位置
	FVector GetMuzzleLocation() const;
	
	//取枪口的方向
	FVector GetMuzzleDirection() const;
	
	//Trace
	FHitResult WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const;
	
	/************************************************************************/
	/* Instigator                                                           */
	/************************************************************************/
protected:

	AFPSCharacter* MyPawn;

public:

	void SetOwningCharacter(AFPSCharacter* NewOwner);

	UFUNCTION(BlueprintCallable, Category = FPSWeapon)
	AFPSCharacter* GetOwningCharacter() const;

	/************************************************************************/
	/* Mesh                                                                     */
	/************************************************************************/
private:

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh1P;
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh3P;

public:
	
	USkeletalMeshComponent* GetWeaponMesh() const;

protected:

	FORCEINLINE USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	FORCEINLINE USkeletalMeshComponent* GetMesh3P() const { return Mesh3P; }

	/************************************************************************/
	/* State                                                                     */
	/************************************************************************/
public:

	EWeaponState::Type GetCurrentState() const;

protected:

	EWeaponState::Type CurrentState;


	void DetermineWeaponState();

	void SetWeaponState(EWeaponState::Type NewState);

	/************************************************************************/
	/* Inventory & Equip                                                                     */
	/************************************************************************/
public:

	bool IsAttachedToPawn() const;

	virtual void OnEnterInventory(AFPSCharacter* NewOwner);

	virtual void OnLeaveInventory();

	virtual void OnEquip(const AFPSWeapon* LastWeapon);

	virtual void OnUnEquip();

protected:

	//装备完成后被调用
	virtual void OnEquipFinished();

private:

	void AttachMeshToPawn();
	void DetachMeshFromPawn();

	uint32 bIsEquipped : 1;
	uint32 bPendingEquip : 1;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FInstigatorAnim EquipAnim;
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* EquipSound;

	//装备动作开始时的时间
	float EquipStartedTime;
	//装备动作的持续时间
	float EquipDurationTime;

	FTimerHandle TimerHandle_OnEquipFinished;

	/************************************************************************/
	/* Animation & Sound                                                    */
	/************************************************************************/
private:

	UAudioComponent* PlayWeaponSound(USoundCue* Sound);

	float PlayChatacterAnimation(const FInstigatorAnim& Animation);

	void StopChatacterAnimation(const FInstigatorAnim& Animation);

	/************************************************************************/
	/* Fire                                                                   */
	/************************************************************************/
public:

	//进入开火状态
	virtual void StartFire();

	//退出开火状态
	virtual void StopFire();

	bool CanFire() const;

protected:

	//开始开火
	virtual void OnBurstStarted();
	//结束开火
	virtual void OnBurstFinished();

	//模拟开火动画，声音，枪口特效
	virtual void StartSimulateWeaponFire();
	virtual void StopSimulatingWeaponFire();

	//最终的开火函数，供子类实现自己的开火逻辑
	virtual void FireWeapon() PURE_VIRTUAL(AFPSWeapon::FireWeapon, );

private:

	//控制开火时的逻辑
	void HandleFiring();

	FTimerHandle TimerHandle_HandleFiring;

	//本轮的开火次数
	int32 BurstCounter;
	//最后一次开火的时间
	float LastFireTime;

	uint32 bWantsToFire : 1;
	uint32 bRefiring;

	//声音
	UAudioComponent* FireAC;

	//如果是循环的声音，那么需要勾选此项
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	uint32 bLoopedFireSound : 1;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireSound;
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireLoopSound;
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireFinishSound;

	//动画
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FInstigatorAnim FireAnim;

	//如果是循环的动画，那么要勾选此项
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	uint32 bLoopedFireAnim : 1;

	//动画是否正在播放
	uint32 bPlayingFireAnim : 1;


	//效果
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	TSubclassOf<UCameraShake> FireCameraShake;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
	uint32 bLoopedMuzzleFX : 1;
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* MuzzleFX;
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName MuzzleAttachPoint;

	UParticleSystemComponent* MuzzlePSC1P;
	UParticleSystemComponent* MuzzlePSC3P;

	/************************************************************************/
	/* Reload                                                                  */
	/************************************************************************/
protected:

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FInstigatorAnim ReloadAnim;
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* ReloadSound;

	uint32 bPendingReload : 1;
	FTimerHandle TimerHandle_StopReload;
	FTimerHandle TimerHandle_ReloadWeapon;

public:

	virtual void StartReload();

	virtual void StopReload();

	virtual void ReloadActual();

	bool CanReload() const;

	/************************************************************************/
	/* Ammo                                                                  */
	/************************************************************************/
private:

	int32 CurrentAmmo;
	int32 CurrentAmmoInClip;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* OutOfAmmoSound;

public:

	bool HasInfiniteClip() const;

	bool HasInfiniteAmmo() const;

	//消耗弹药
	void ConsumeAmmo();

	//增添弹药
	void AddAmmo(int AddAmount);

	//取当前剩余弹药总量
	UFUNCTION(BlueprintCallable, Category = FPSWeapon)
	int32 GetCurrentAmmo() const;

	//取当前弹夹内剩余的弹药量
	UFUNCTION(BlueprintCallable, Category = FPSWeapon)
	int32 GetCurrentAmmoInClip() const;

	//取每弹夹的弹药容量
	int32 GetAmmoPerClip() const;

	//取武器最大弹药容量
	int32 GetMaxAmmo() const;

	//取当前的弹药类型
	FORCEINLINE virtual EAmmoType GetAmmoType() const { return EAmmoType::EBullet; }
};
