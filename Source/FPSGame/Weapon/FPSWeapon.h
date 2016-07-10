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

	//�Ƿ����޵�ҩ
	UPROPERTY(EditDefaultsOnly, Category = WeaponData)
	bool bInfiniteAmmo;

	//�Ƿ����޵���
	UPROPERTY(EditDefaultsOnly, Category = WeaponData)
	bool bInfiniteClip;

	//���ҩ����
	UPROPERTY(EditDefaultsOnly, Category = WeaponData)
	int32 MaxAmmo;

	//ÿ�����ڵ��ӵ���
	UPROPERTY(EditDefaultsOnly, Category = WeaponData)
	int32 AmmoPerClip;

	//��ʼ������
	UPROPERTY(EditDefaultsOnly, Category = WeaponData)
	int32 InitialClips;

	//����״̬�µ�ÿ��������ʱ��
	UPROPERTY(EditDefaultsOnly, Category = WeaponData)
	float TimeBetweenShots;

	//û��װ�������������Ҫ��װ�����ʱ��
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

	//ȡ��ǰ��ɫ��׼�ķ���
	virtual FVector GetAdjustedAimDirection() const;

	//ȡ��ǰ�������ʼλ��
	FVector GetShootStartLocation(const FVector& AimDir) const;

	//ȡǹ�ڵ�λ��
	FVector GetMuzzleLocation() const;
	
	//ȡǹ�ڵķ���
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

	//װ����ɺ󱻵���
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

	//װ��������ʼʱ��ʱ��
	float EquipStartedTime;
	//װ�������ĳ���ʱ��
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

	//���뿪��״̬
	virtual void StartFire();

	//�˳�����״̬
	virtual void StopFire();

	bool CanFire() const;

protected:

	//��ʼ����
	virtual void OnBurstStarted();
	//��������
	virtual void OnBurstFinished();

	//ģ�⿪�𶯻���������ǹ����Ч
	virtual void StartSimulateWeaponFire();
	virtual void StopSimulatingWeaponFire();

	//���յĿ�������������ʵ���Լ��Ŀ����߼�
	virtual void FireWeapon() PURE_VIRTUAL(AFPSWeapon::FireWeapon, );

private:

	//���ƿ���ʱ���߼�
	void HandleFiring();

	FTimerHandle TimerHandle_HandleFiring;

	//���ֵĿ������
	int32 BurstCounter;
	//���һ�ο����ʱ��
	float LastFireTime;

	uint32 bWantsToFire : 1;
	uint32 bRefiring;

	//����
	UAudioComponent* FireAC;

	//�����ѭ������������ô��Ҫ��ѡ����
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	uint32 bLoopedFireSound : 1;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireSound;
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireLoopSound;
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireFinishSound;

	//����
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FInstigatorAnim FireAnim;

	//�����ѭ���Ķ�������ôҪ��ѡ����
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	uint32 bLoopedFireAnim : 1;

	//�����Ƿ����ڲ���
	uint32 bPlayingFireAnim : 1;


	//Ч��
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

	//���ĵ�ҩ
	void ConsumeAmmo();

	//����ҩ
	void AddAmmo(int AddAmount);

	//ȡ��ǰʣ�൯ҩ����
	UFUNCTION(BlueprintCallable, Category = FPSWeapon)
	int32 GetCurrentAmmo() const;

	//ȡ��ǰ������ʣ��ĵ�ҩ��
	UFUNCTION(BlueprintCallable, Category = FPSWeapon)
	int32 GetCurrentAmmoInClip() const;

	//ȡÿ���еĵ�ҩ����
	int32 GetAmmoPerClip() const;

	//ȡ�������ҩ����
	int32 GetMaxAmmo() const;

	//ȡ��ǰ�ĵ�ҩ����
	FORCEINLINE virtual EAmmoType GetAmmoType() const { return EAmmoType::EBullet; }
};
