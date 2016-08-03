// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "FPSWeapon.h"
#include "Player/FPSCharacter.h"
#include "Player/FPSPlayerController.h"
#include "Bot/FPSAIController.h"
#include "Player/FPSPlayerState.h"
#include "UI/GameHUD.h"
#include "Item/ItemWeapon.h"

// Sets default values
AFPSWeapon::AFPSWeapon()
{
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh1P"));
	Mesh1P->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	Mesh1P->bReceivesDecals = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh1P->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = Mesh1P;

	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh3P"));
	Mesh3P->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	Mesh3P->bReceivesDecals = false;
	Mesh3P->CastShadow = true;
	Mesh3P->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh3P->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh3P->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	Mesh3P->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Mesh3P->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	CurrentAmmo = 0;
	CurrentAmmoInClip = 0;
	BurstCounter = 0;
	LastFireTime = 0.0f;

	bLoopedMuzzleFX = false;
	bLoopedFireAnim = false;
	bPlayingFireAnim = false;
	bIsEquipped = false;
	bWantsToFire = false;
	bPendingReload = false;
	bPendingEquip = false;
	CurrentState = EWeaponState::Idle;


	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
}


void AFPSWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (WeaponConfig.InitialClips > 0)
	{
		CurrentAmmoInClip = WeaponConfig.AmmoPerClip;
		CurrentAmmo = WeaponConfig.AmmoPerClip * WeaponConfig.InitialClips;
	}

	DetachMeshFromPawn();
}

void AFPSWeapon::Destroyed()
{
	Super::Destroyed();

	StopSimulatingWeaponFire();
}

FVector AFPSWeapon::GetAdjustedAimDirection() const
{
	AFPSPlayerController* const PlayerController = Instigator ? Cast<AFPSPlayerController>(Instigator->Controller) : NULL;
	FVector FinalAim = FVector::ZeroVector;

	if (PlayerController)
	{
		FVector CamLoc;
		FRotator CamRot;
		PlayerController->GetPlayerViewPoint(CamLoc, CamRot);
		FinalAim = CamRot.Vector();
	}
	else if (Instigator)
	{
		AFPSAIController* AIController = MyPawn ? Cast<AFPSAIController>(MyPawn->Controller) : NULL;
		if (AIController != NULL)
		{
			FinalAim = AIController->GetControlRotation().Vector();
		}
		else
		{
			FinalAim = Instigator->GetBaseAimRotation().Vector();
		}
	}

	return FinalAim;
}

float AFPSWeapon::GetCurrentSpread() const
{
	float FinalSpread = WeaponConfig.WeaponSpread + CurrentFiringSpread;
	if (MyPawn && MyPawn->IsTargeting())
	{
		FinalSpread *= WeaponConfig.TargetingSpreadMod;
	}

	return FinalSpread;
}

float AFPSWeapon::GetSpreadMax() const
{
	return WeaponConfig.FiringSpreadMax;
}

void AFPSWeapon::IncreaseSpread()
{
	CurrentFiringSpread = FMath::Min(WeaponConfig.FiringSpreadMax, CurrentFiringSpread + WeaponConfig.FiringSpreadIncrement);
}

void AFPSWeapon::ResetSpread()
{
	CurrentFiringSpread = 0.0f;
}

UTexture2D* AFPSWeapon::GetItemIcon() const
{
	
	UTexture2D* Icon = ItemClass->GetDefaultObject<AItemWeapon>()->GetIcon();
	if (Icon)
	{
		return Icon;
	}
	return nullptr;
}

FVector AFPSWeapon::GetCameraAim() const
{
	AFPSPlayerController* const PlayerController = Instigator ? Cast<AFPSPlayerController>(Instigator->Controller) : NULL;
	FVector FinalAim = FVector::ZeroVector;

	if (PlayerController)
	{
		FVector CamLoc;
		FRotator CamRot;
		PlayerController->GetPlayerViewPoint(CamLoc, CamRot);
		FinalAim = CamRot.Vector();
	}
	else if (Instigator)
	{
		FinalAim = Instigator->GetBaseAimRotation().Vector();
	}

	return FinalAim;
}

FVector AFPSWeapon::GetShootStartLocation(const FVector& AimDir) const
{
	AFPSPlayerController* PC = MyPawn ? Cast<AFPSPlayerController>(MyPawn->Controller) : NULL;
	AFPSAIController* AIPC = MyPawn ? Cast<AFPSAIController>(MyPawn->Controller) : NULL;
	FVector OutStartTrace = FVector::ZeroVector;

	if (PC)
	{
		FRotator UnusedRot;
		PC->GetPlayerViewPoint(OutStartTrace, UnusedRot);
		OutStartTrace = OutStartTrace + AimDir * (FVector::DotProduct((Instigator->GetActorLocation() - OutStartTrace), AimDir));
	}
	else if (AIPC)
	{
		OutStartTrace = GetMuzzleLocation();
	}

	return OutStartTrace;
}

FVector AFPSWeapon::GetMuzzleLocation() const
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketLocation(MuzzleAttachPoint);
}

FVector AFPSWeapon::GetMuzzleDirection() const
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketRotation(MuzzleAttachPoint).Vector();
}

FHitResult AFPSWeapon::WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const
{
	static FName WeaponFireTag = FName(TEXT("WeaponTrace"));

	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(WeaponFireTag, true, Instigator);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, TraceFrom, TraceTo, COLLISION_WEAPON, TraceParams);

	return Hit;
}

void AFPSWeapon::SetOwningCharacter(AFPSCharacter* NewOwner)
{
	if (MyPawn != NewOwner)
	{
		Instigator = NewOwner;
		MyPawn = NewOwner;
		SetOwner(NewOwner);
	}
}

AFPSCharacter* AFPSWeapon::GetOwningCharacter() const
{
	return MyPawn;
}

USkeletalMeshComponent* AFPSWeapon::GetWeaponMesh() const
{
	return (MyPawn != NULL && MyPawn->IsFirstPerson()) ? Mesh1P : Mesh3P;
}


EWeaponState::Type AFPSWeapon::GetCurrentState() const
{
	return CurrentState;
}

void AFPSWeapon::DetermineWeaponState()
{
	EWeaponState::Type NewState = EWeaponState::Idle;

	if (bIsEquipped)
	{
		if (bPendingReload)
		{
			if (CanReload() == false)
			{
				NewState = CurrentState;
			}
			else
			{
				NewState = EWeaponState::Reloading;
			}
		}
		else if ((bPendingReload == false) && (bWantsToFire == true) && (CanFire() == true))
		{
			NewState = EWeaponState::Firing;
		}
	}
	else if (bPendingEquip)
	{
		NewState = EWeaponState::Equipping;
	}

	SetWeaponState(NewState);
}

void AFPSWeapon::SetWeaponState(EWeaponState::Type NewState)
{
	const EWeaponState::Type PrevState = CurrentState;

	if (PrevState == EWeaponState::Firing && NewState != EWeaponState::Firing)
	{
		OnBurstFinished();
	}

	CurrentState = NewState;

	if (PrevState != EWeaponState::Firing && NewState == EWeaponState::Firing)
	{
		OnBurstStarted();
	}
}

void AFPSWeapon::OnEnterInventory(AFPSCharacter* NewOwner)
{
	SetOwningCharacter(NewOwner);
}

void AFPSWeapon::OnLeaveInventory()
{
	SetOwningCharacter(NULL);

	if (IsAttachedToPawn())
	{
		OnUnEquip();
	}
}

bool AFPSWeapon::IsAttachedToPawn() const
{
	return bIsEquipped || bPendingEquip;
}

void AFPSWeapon::OnEquip(const AFPSWeapon* LastWeapon)
{
	AttachMeshToPawn();

	bPendingEquip = true;
	DetermineWeaponState();

	float Duration = PlayChatacterAnimation(EquipAnim);
	if (Duration <= 0.0f)
	{
		// failsafe
		Duration = 0.5f;
	}
	EquipStartedTime = GetWorld()->GetTimeSeconds();
	EquipDurationTime = Duration;

	GetWorldTimerManager().SetTimer(TimerHandle_OnEquipFinished, this, &AFPSWeapon::OnEquipFinished, Duration, false);

	if (MyPawn)
	{
		PlayWeaponSound(EquipSound);
	}
}

void AFPSWeapon::OnUnEquip()
{
	DetachMeshFromPawn();
	bIsEquipped = false;
	StopFire();

	if (bPendingReload)
	{
		StopChatacterAnimation(ReloadAnim);
		bPendingReload = false;

		GetWorldTimerManager().ClearTimer(TimerHandle_StopReload);
		GetWorldTimerManager().ClearTimer(TimerHandle_ReloadWeapon);
	}

	if (bPendingEquip)
	{
		StopChatacterAnimation(EquipAnim);
		bPendingEquip = false;

		GetWorldTimerManager().ClearTimer(TimerHandle_OnEquipFinished);
	}

	DetermineWeaponState();
}

void AFPSWeapon::OnEquipFinished()
{
	AttachMeshToPawn();

	bIsEquipped = true;
	bPendingEquip = false;

	// Determine the state so that the can reload checks will work
	DetermineWeaponState();

	if (MyPawn)
	{
		// try to reload empty clip
		if (CurrentAmmoInClip <= 0 &&
			CanReload())
		{
			StartReload();
		}
	}
}

void AFPSWeapon::AttachMeshToPawn()
{
	if (MyPawn)
	{
		// Remove and hide both first and third person meshes
		DetachMeshFromPawn();

		// For locally controller players we attach both weapons and let the bOnlyOwnerSee, bOwnerNoSee flags deal with visibility.
		FName AttachPoint = MyPawn->GetWeaponAttachPoint();
		USkeletalMeshComponent* PawnMesh1p = MyPawn->GetSpecifcPawnMesh(true);
		USkeletalMeshComponent* PawnMesh3p = MyPawn->GetSpecifcPawnMesh(false);
		Mesh1P->SetHiddenInGame(false);
		Mesh3P->SetHiddenInGame(false);
		Mesh1P->AttachTo(PawnMesh1p, AttachPoint, EAttachLocation::SnapToTarget);
		Mesh3P->AttachTo(PawnMesh3p, AttachPoint, EAttachLocation::SnapToTarget);
		Mesh3P->bOwnerNoSee = true;
		Mesh1P->bOnlyOwnerSee = true;

	}
}

void AFPSWeapon::DetachMeshFromPawn()
{
	Mesh1P->DetachFromParent();
	Mesh1P->SetHiddenInGame(true);

	Mesh3P->DetachFromParent();
	Mesh3P->SetHiddenInGame(true);
}

void AFPSWeapon::StartFire()
{

	if (!bWantsToFire)
	{
		bWantsToFire = true;
		DetermineWeaponState();
	}
}

void AFPSWeapon::StopFire()
{

	if (bWantsToFire)
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}
}

bool AFPSWeapon::CanFire() const
{
	bool bCanFire = MyPawn && MyPawn->CanFire();
	bool bStateOKToFire = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
	return ((bCanFire == true) && (bStateOKToFire == true) && (bPendingReload == false));
}

UAudioComponent* AFPSWeapon::PlayWeaponSound(USoundCue* Sound)
{
	UAudioComponent* AC = NULL;

	if (Sound && MyPawn)
	{
		AC = UGameplayStatics::SpawnSoundAttached(Sound, MyPawn->GetRootComponent());
	}

	return AC;
}

float AFPSWeapon::PlayChatacterAnimation(const FInstigatorAnim& Animation)
{
	float Duration = 0.0f;
	if (MyPawn)
	{
		UAnimMontage* UseAnim = MyPawn->IsFirstPerson() ? Animation.Pawn1P : Animation.Pawn3P;
		if (UseAnim)
		{
			Duration = MyPawn->PlayAnimMontage(UseAnim);
		}
	}

	return Duration;
}

void AFPSWeapon::StopChatacterAnimation(const FInstigatorAnim& Animation)
{
	if (MyPawn)
	{
		UAnimMontage* UseAnim = MyPawn->IsFirstPerson() ? Animation.Pawn1P : Animation.Pawn3P;
		if (UseAnim)
		{
			MyPawn->StopAnimMontage(UseAnim);
		}
	}
}

void AFPSWeapon::HandleFiring()
{
	//有弹药 可以直接开火
	if ((CurrentAmmoInClip > 0 || HasInfiniteClip() || HasInfiniteAmmo()) && CanFire())
	{
		StartSimulateWeaponFire();

		if (MyPawn)
		{
			FireWeapon();

			ConsumeAmmo();

			BurstCounter++;
		}

	}
	//弹夹内没弹药了，但可以Reload
	else if (CanReload())
	{
		StartReload();
	}
	//弹夹内没弹药了，也不能Reload，不能继续开火了
	else if (MyPawn)
	{
		if (GetCurrentAmmo() == 0 && !bRefiring)
		{
			PlayWeaponSound(OutOfAmmoSound);
			AFPSPlayerController* MyPC = Cast<AFPSPlayerController>(MyPawn->Controller);
			AGameHUD* MyHUD = MyPC ? Cast<AGameHUD>(MyPC->GetHUD()) : NULL;
			if (MyHUD)
			{
				MyHUD->NotifyOutOfAmmo();
			}
		}

		if (BurstCounter > 0)
		{
			OnBurstFinished();
		}
	}


	if (MyPawn)
	{
		//弹药用光，Reload
		if (CurrentAmmoInClip <= 0 && CanReload())
		{
			StartReload();
		}

		//继续本轮射击
		bRefiring = (CurrentState == EWeaponState::Firing && WeaponConfig.TimeBetweenShots > 0.0f);
		if (bRefiring)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &AFPSWeapon::HandleFiring, WeaponConfig.TimeBetweenShots, false);
		}
	}
	LastFireTime = GetWorld()->GetTimeSeconds();
}

void AFPSWeapon::OnBurstStarted()
{
	// start firing, can be delayed to satisfy TimeBetweenShots
	const float GameTime = GetWorld()->GetTimeSeconds();
	if (LastFireTime > 0 && WeaponConfig.TimeBetweenShots > 0.0f &&
		LastFireTime + WeaponConfig.TimeBetweenShots > GameTime)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &AFPSWeapon::HandleFiring, LastFireTime + WeaponConfig.TimeBetweenShots - GameTime, false);
	}
	else
	{
		HandleFiring();
	}
}

void AFPSWeapon::OnBurstFinished()
{
	// stop firing FX on remote clients
	BurstCounter = 0;

	StopSimulatingWeaponFire();

	GetWorldTimerManager().ClearTimer(TimerHandle_HandleFiring);
	bRefiring = false;
}

void AFPSWeapon::StartSimulateWeaponFire()
{
	if (CurrentState != EWeaponState::Firing)
	{
		return;
	}

	//枪口特效
	if (MuzzleFX)
	{
		USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
		if (!bLoopedMuzzleFX || MuzzlePSC1P == NULL)
		{
			if (MyPawn != NULL)
			{
				AController* PlayerCon = MyPawn->GetController();
				if (PlayerCon != NULL)
				{
					//第一人称
					Mesh1P->GetSocketLocation(MuzzleAttachPoint);
					MuzzlePSC1P = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh1P, MuzzleAttachPoint);
					MuzzlePSC1P->bOwnerNoSee = false;
					MuzzlePSC1P->bOnlyOwnerSee = true;

					//第三人称
					Mesh3P->GetSocketLocation(MuzzleAttachPoint);
					MuzzlePSC3P = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh3P, MuzzleAttachPoint);
					MuzzlePSC3P->bOwnerNoSee = true;
					MuzzlePSC3P->bOnlyOwnerSee = false;
				}
			}
			else
			{
				MuzzlePSC1P = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, UseWeaponMesh, MuzzleAttachPoint);
			}

		}
	}

	//Owner的动画
	if (!bLoopedFireAnim || !bPlayingFireAnim)
	{
		PlayChatacterAnimation(FireAnim);
		bPlayingFireAnim = true;
	}

	//声音
	if (bLoopedFireSound)
	{
		if (FireAC == NULL)
		{
			FireAC = PlayWeaponSound(FireLoopSound);
		}
	}
	else
	{
		PlayWeaponSound(FireSound);
	}
	
	//CameraShake
	AFPSPlayerController* PC = (MyPawn != NULL) ? Cast<AFPSPlayerController>(MyPawn->Controller) : NULL;
	if (PC != NULL)
	{
		if (FireCameraShake != NULL)
		{
			PC->ClientPlayCameraShake(FireCameraShake, 1);
		}
	}
}

void AFPSWeapon::StopSimulatingWeaponFire()
{
	//枪口特效
	if (bLoopedMuzzleFX)
	{
		if (MuzzlePSC1P != NULL)
		{
			MuzzlePSC1P->DeactivateSystem();
			MuzzlePSC1P = NULL;
		}
		if (MuzzlePSC3P != NULL)
		{
			MuzzlePSC3P->DeactivateSystem();
			MuzzlePSC3P = NULL;
		}
	}

	//Owner动画
	if (bLoopedFireAnim && bPlayingFireAnim)
	{
		StopChatacterAnimation(FireAnim);
		bPlayingFireAnim = false;
	}

	//声音
	if (FireAC)
	{
		FireAC->FadeOut(0.1f, 0.0f);
		FireAC = NULL;

		PlayWeaponSound(FireFinishSound);
	}
}

void AFPSWeapon::StartReload()
{
	if (CanReload())
	{
		bPendingReload = true;
		DetermineWeaponState();

		float AnimDuration = PlayChatacterAnimation(ReloadAnim);
		if (AnimDuration <= 0.0f)
		{
			AnimDuration = WeaponConfig.NoAnimReloadDuration;
		}

		GetWorldTimerManager().SetTimer(TimerHandle_StopReload, this, &AFPSWeapon::StopReload, AnimDuration, false);
		GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon, this, &AFPSWeapon::ReloadActual, FMath::Max(0.1f, AnimDuration - 0.1f), false);

		if (MyPawn)
		{
			PlayWeaponSound(ReloadSound);
		}
	}
}

void AFPSWeapon::StopReload()
{
	if (CurrentState == EWeaponState::Reloading)
	{
		bPendingReload = false;
		DetermineWeaponState();
		StopChatacterAnimation(ReloadAnim);
	}
}


void AFPSWeapon::ReloadActual()
{
	int32 ClipDelta = FMath::Min(WeaponConfig.AmmoPerClip - CurrentAmmoInClip, CurrentAmmo - CurrentAmmoInClip);

	if (HasInfiniteClip())
	{
		ClipDelta = WeaponConfig.AmmoPerClip - CurrentAmmoInClip;
	}

	if (ClipDelta > 0)
	{
		CurrentAmmoInClip += ClipDelta;
	}

	if (HasInfiniteClip())
	{
		CurrentAmmo = FMath::Max(CurrentAmmoInClip, CurrentAmmo);
	}
}


bool AFPSWeapon::CanReload() const
{
	bool bCanReload = (!MyPawn || MyPawn->CanReload());
	bool bGotAmmo = (CurrentAmmoInClip < WeaponConfig.AmmoPerClip) && (CurrentAmmo - CurrentAmmoInClip > 0 || HasInfiniteClip());
	bool bStateOKToReload = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
	return ((bCanReload == true) && (bGotAmmo == true) && (bStateOKToReload == true));
}

bool AFPSWeapon::HasInfiniteClip() const
{
	const AFPSPlayerController* MyPC = (MyPawn != NULL) ? Cast<const AFPSPlayerController>(MyPawn->Controller) : NULL;
	return WeaponConfig.bInfiniteClip || (MyPC && MyPC->HasInfiniteClip());
}

bool AFPSWeapon::HasInfiniteAmmo() const
{
	const AFPSPlayerController* MyPC = (MyPawn != NULL) ? Cast<const AFPSPlayerController>(MyPawn->Controller) : NULL;
	return WeaponConfig.bInfiniteAmmo || (MyPC && MyPC->HasInfiniteAmmo());
}

void AFPSWeapon::ConsumeAmmo()
{
	if (!HasInfiniteAmmo())
	{
		CurrentAmmoInClip--;
	}

	if (!HasInfiniteAmmo() && !HasInfiniteClip())
	{
		CurrentAmmo--;
	}

	AFPSAIController* BotAI = MyPawn ? Cast<AFPSAIController>(MyPawn->GetController()) : NULL;
	AFPSPlayerController* PlayerController = MyPawn ? Cast<AFPSPlayerController>(MyPawn->GetController()) : NULL;
	if (BotAI)
	{
		BotAI->CheckAmmo(this);
	}
	else if (PlayerController)
	{
		AFPSPlayerState* PlayerState = Cast<AFPSPlayerState>(PlayerController->PlayerState);
		switch (GetAmmoType())
		{
		case EAmmoType::ERocket:
			PlayerState->AddRocketsFired(1);
			break;
		case EAmmoType::EBullet:
		default:
			PlayerState->AddBulletsFired(1);
			break;
		}
	}
}

void AFPSWeapon::AddAmmo(int AddAmount)
{
	const int32 MissingAmmo = FMath::Max(0, WeaponConfig.MaxAmmo - CurrentAmmo);
	AddAmount = FMath::Min(AddAmount, MissingAmmo);
	CurrentAmmo += AddAmount;

	AFPSAIController* BotAI = MyPawn ? Cast<AFPSAIController>(MyPawn->GetController()) : NULL;
	if (BotAI)
	{
		BotAI->CheckAmmo(this);
	}

	// start reload if clip was empty
	if (GetCurrentAmmoInClip() <= 0 &&
		CanReload() &&
		MyPawn->GetWeapon() == this)
	{
		StartReload();
	}
}

int32 AFPSWeapon::GetCurrentAmmo() const
{
	return CurrentAmmo;
}

int32 AFPSWeapon::GetCurrentAmmoInClip() const
{
	return CurrentAmmoInClip;
}

int32 AFPSWeapon::GetAmmoPerClip() const
{
	return WeaponConfig.AmmoPerClip;
}

int32 AFPSWeapon::GetMaxAmmo() const
{
	return WeaponConfig.MaxAmmo;
}