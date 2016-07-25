// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "FPSCharacter.h"
#include "Weapon/FPSWeapon.h"
#include "Player/FPSPlayerState.h"
#include "Player/FPSCharacterMovementComponent.h"
#include "Player/FPSPlayerController.h"
#include "GameMode/FPSGameMode.h"
#include "UI/GameHUD.h"
#include "Weapon/FPSDamageType.h"
#include "Gameplay/UsableActor.h"
#include "Item/ItemWeapon.h"

AFPSCharacter::AFPSCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UFPSCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	Mesh1P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("PawnMesh1P"));
	Mesh1P->AttachParent = GetCapsuleComponent();
	Mesh1P->bOnlyOwnerSee = true;
	Mesh1P->bOwnerNoSee = false;
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->bReceivesDecals = false;
	Mesh1P->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	Mesh1P->PrimaryComponentTick.TickGroup = TG_PrePhysics;
	Mesh1P->SetCollisionObjectType(ECC_Pawn);
	Mesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh1P->SetCollisionResponseToAllChannels(ECR_Ignore);

	GetMesh()->bOnlyOwnerSee = false;
	GetMesh()->bOwnerNoSee = true;
	GetMesh()->bReceivesDecals = false;
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);
	
	bIsTargeting = false;
	bWantsToRun = false;
	bWantsToFire = false;
	LowHealthPercentage = 0.5f;

	RunningSpeedModifier = 1.5f;
	TargetingSpeedModifier = 0.5f;

}


void AFPSCharacter::BeginPlay()
{
	Super::BeginPlay();

	Health = GetMaxHealth();
	SpawnDefaultInventory();


	// set initial mesh visibility (3rd person view)
	UpdatePawnMeshes();

	// create material instance for setting team colors (3rd person view)
	for (int32 iMat = 0; iMat < GetMesh()->GetNumMaterials(); iMat++)
	{
		MeshMIDs.Add(GetMesh()->CreateAndSetMaterialInstanceDynamic(iMat));
	}

	if (RespawnFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, RespawnFX, GetActorLocation(), GetActorRotation());
	}

	if (RespawnSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, RespawnSound, GetActorLocation());
	}
}

void AFPSCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if (bWantsToRunToggled && !IsRunning())
	{
		SetRunning(false, false);
	}

	AFPSPlayerController* MyPC = Cast<AFPSPlayerController>(Controller);
	if (MyPC && MyPC->HasHealthRegen())
	{
		if (this->Health < this->GetMaxHealth())
		{
			this->Health += 5 * DeltaTime;
			if (Health > this->GetMaxHealth())
			{
				Health = this->GetMaxHealth();
			}
		}
	}

	if (LowHealthSound && GEngine->UseSound())
	{
		if ((this->Health > 0 && this->Health < this->GetMaxHealth() * LowHealthPercentage) && (!LowHealthWarningPlayer || !LowHealthWarningPlayer->IsPlaying()))
		{
			LowHealthWarningPlayer = UGameplayStatics::SpawnSoundAttached(LowHealthSound, GetRootComponent(),
				NAME_None, FVector(ForceInit), EAttachLocation::KeepRelativeOffset, true);
			LowHealthWarningPlayer->SetVolumeMultiplier(0.0f);
		}
		else if ((this->Health > this->GetMaxHealth() * LowHealthPercentage || this->Health < 0) && LowHealthWarningPlayer && LowHealthWarningPlayer->IsPlaying())
		{
			LowHealthWarningPlayer->Stop();
		}
		if (LowHealthWarningPlayer && LowHealthWarningPlayer->IsPlaying())
		{
			const float MinVolume = 0.3f;
			const float VolumeMultiplier = (1.0f - (this->Health / (this->GetMaxHealth() * LowHealthPercentage)));
			LowHealthWarningPlayer->SetVolumeMultiplier(MinVolume + (1.0f - MinVolume) * VolumeMultiplier);
		}
	}

	TickViewUsable();
}

void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* InputComponent)
{
	check(InputComponent);
	InputComponent->BindAxis("MoveForward", this, &AFPSCharacter::OnMoveForward);
	InputComponent->BindAxis("MoveRight", this, &AFPSCharacter::OnMoveRight);
	InputComponent->BindAxis("MoveUp", this, &AFPSCharacter::OnMoveUp);
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	InputComponent->BindAction("Fire", IE_Pressed, this, &AFPSCharacter::OnStartFire);
	InputComponent->BindAction("Fire", IE_Released, this, &AFPSCharacter::OnStopFire);

	InputComponent->BindAction("Targeting", IE_Pressed, this, &AFPSCharacter::OnStartTargeting);
	InputComponent->BindAction("Targeting", IE_Released, this, &AFPSCharacter::OnStopTargeting);

	InputComponent->BindAction("NextWeapon", IE_Pressed, this, &AFPSCharacter::OnNextWeapon);
	InputComponent->BindAction("PrevWeapon", IE_Pressed, this, &AFPSCharacter::OnPrevWeapon);

	InputComponent->BindAction("Reload", IE_Pressed, this, &AFPSCharacter::OnReload);

	InputComponent->BindAction("Jump", IE_Pressed, this, &AFPSCharacter::OnStartJump);
	InputComponent->BindAction("Jump", IE_Released, this, &AFPSCharacter::OnStopJump);

	InputComponent->BindAction("Run", IE_Pressed, this, &AFPSCharacter::OnStartRunning);
	InputComponent->BindAction("Run", IE_Released, this, &AFPSCharacter::OnStopRunning);

	InputComponent->BindAction("Use", IE_Pressed, this, &AFPSCharacter::OnUse);
	InputComponent->BindAction("Drop", IE_Pressed, this, &AFPSCharacter::OnDrop);
}

void AFPSCharacter::OnMoveForward(float Val)
{
	if (Controller && Val != 0.f)
	{
		// Limit pitch when walking or falling
		const bool bLimitRotation = (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling());
		const FRotator Rotation = bLimitRotation ? GetActorRotation() : Controller->GetControlRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
		AddMovementInput(Direction, Val);
	}
}

void AFPSCharacter::OnMoveRight(float Val)
{
	if (Val != 0.f)
	{
		const FQuat Rotation = GetActorQuat();
		const FVector Direction = FQuatRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
		AddMovementInput(Direction, Val);
	}
}

void AFPSCharacter::OnMoveUp(float Val)
{
	if (Val != 0.f)
	{
		// Not when walking or falling.
		if (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling())
		{
			return;
		}

		AddMovementInput(FVector::UpVector, Val);
	}
}

void AFPSCharacter::OnStartFire()
{
	AFPSPlayerController* MyPC = Cast<AFPSPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		if (IsRunning())
		{
			SetRunning(false, false);
		}
		StartWeaponFire();
	}
}

void AFPSCharacter::OnStopFire()
{
	StopWeaponFire();
}

void AFPSCharacter::OnStartTargeting()
{
	AFPSPlayerController* MyPC = Cast<AFPSPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		if (IsRunning())
		{
			SetRunning(false, false);
		}
		SetTargeting(true);
	}
}

void AFPSCharacter::OnStopTargeting()
{
	SetTargeting(false);
}

void AFPSCharacter::OnNextWeapon()
{
	AFPSPlayerController* MyPC = Cast<AFPSPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		if (Inventory.Num() >= 2 && (WeaponCurrent == NULL || WeaponCurrent->GetCurrentState() != EWeaponState::Equipping))
		{
			const int32 CurrentWeaponIdx = Inventory.IndexOfByKey(WeaponCurrent);
			AFPSWeapon* NextWeapon = Inventory[(CurrentWeaponIdx + 1) % Inventory.Num()];
			EquipWeapon(NextWeapon);
		}
	}
}

void AFPSCharacter::OnPrevWeapon()
{
	AFPSPlayerController* MyPC = Cast<AFPSPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		if (Inventory.Num() >= 2 && (WeaponCurrent == NULL || WeaponCurrent->GetCurrentState() != EWeaponState::Equipping))
		{
			const int32 CurrentWeaponIdx = Inventory.IndexOfByKey(WeaponCurrent);
			AFPSWeapon* PrevWeapon = Inventory[(CurrentWeaponIdx - 1 + Inventory.Num()) % Inventory.Num()];
			EquipWeapon(PrevWeapon);
		}
	}
}

void AFPSCharacter::OnReload()
{
	AFPSPlayerController* MyPC = Cast<AFPSPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		if (WeaponCurrent)
		{
			WeaponCurrent->StartReload();
		}
	}
}

void AFPSCharacter::OnStartJump()
{
	AFPSPlayerController* MyPC = Cast<AFPSPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		bPressedJump = true;
	}
}

void AFPSCharacter::OnStopJump()
{
	bPressedJump = false;
}

void AFPSCharacter::OnStartRunning()
{
	AFPSPlayerController* MyPC = Cast<AFPSPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		if (IsTargeting())
		{
			SetTargeting(false);
		}
		StopWeaponFire();
		SetRunning(true, false);
	}
}

void AFPSCharacter::OnStopRunning()
{
	SetRunning(false, false);
}

void AFPSCharacter::OnUse()
{
	AUsableActor* TheUsableActor = GetUsableActorInView();
	if (TheUsableActor)
	{
		TheUsableActor->OnUsed(this);
	}
}

void AFPSCharacter::OnDrop()
{
	if (WeaponCurrent)
	{
		if (Controller == nullptr)
		{
			return;
		}
		
		//丢之前 检测一下丢的方向会不会阻挡，如果会，那么丢的近一些
		FVector CamLoc;
		FRotator CamRot;
		Controller->GetPlayerViewPoint(CamLoc, CamRot);

		FVector SpawnLoc;
		FRotator SpawnRot = CamRot;
		const FVector Direction = CamRot.Vector();
		const FVector TraceStart = GetActorLocation();
		const FVector TraceEnd = GetActorLocation() + (Direction * DropWeaponMaxDistance);

		FCollisionQueryParams TraceParam;
		TraceParam.bTraceComplex = false;
		TraceParam.bReturnPhysicalMaterial = false;
		TraceParam.AddIgnoredActor(this);
		FHitResult Hit;
		GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldDynamic, TraceParam);

		if (Hit.bBlockingHit)
		{
			SpawnLoc = Hit.ImpactPoint + (Hit.ImpactNormal * 20);
		}
		else
		{
			SpawnLoc = TraceEnd;
		}

		FActorSpawnParameters SpawnParam;
		SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AItemWeapon* NewWeaponItem = GetWorld()->SpawnActor<AItemWeapon>(WeaponCurrent->ItemClass, SpawnLoc, FRotator::ZeroRotator, SpawnParam);

		if (NewWeaponItem)
		{
			
			UMeshComponent* MeshComp = NewWeaponItem->GetMeshComponent();
			if (MeshComp)
			{
				MeshComp->SetSimulatePhysics(true);
				MeshComp->AddTorque(FVector(1, 1, 1) * 400000);
				MeshComp->AddForce(Direction * 400000);
			}
		}

		RemoveWeapon(WeaponCurrent);
		UnEquipWeapon(WeaponCurrent);

		if (Inventory.Num() > 0)
		{
			SetCurrentWeapon(Inventory[0]);
		}

		if (Inventory.Num() == 0)
		{
			SetCurrentWeapon(nullptr);
		}
	}
}


void AFPSCharacter::Destroyed()
{
	Super::Destroyed();
	DestroyInventory();
}


void AFPSCharacter::OnCameraUpdate(const FVector& CameraLocation, const FRotator& CameraRotation)
{
	USkeletalMeshComponent* DefMesh1P = Cast<USkeletalMeshComponent>(GetClass()->GetDefaultSubobjectByName(TEXT("PawnMesh1P")));
	const FMatrix DefMeshLS = FRotationTranslationMatrix(DefMesh1P->RelativeRotation, DefMesh1P->RelativeLocation);
	const FMatrix LocalToWorld = ActorToWorld().ToMatrixWithScale();

	// Mesh rotating code expect uniform scale in LocalToWorld matrix

	const FRotator RotCameraPitch(CameraRotation.Pitch, 0.0f, 0.0f);
	const FRotator RotCameraYaw(0.0f, CameraRotation.Yaw, 0.0f);

	const FMatrix LeveledCameraLS = FRotationTranslationMatrix(RotCameraYaw, CameraLocation) * LocalToWorld.Inverse();
	const FMatrix PitchedCameraLS = FRotationMatrix(RotCameraPitch) * LeveledCameraLS;
	const FMatrix MeshRelativeToCamera = DefMeshLS * LeveledCameraLS.Inverse();
	const FMatrix PitchedMesh = MeshRelativeToCamera * PitchedCameraLS;

	Mesh1P->SetRelativeLocationAndRotation(PitchedMesh.GetOrigin(), PitchedMesh.Rotator());
}

FRotator AFPSCharacter::GetAimOffsets() const
{
	const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLS = AimDirLS.Rotation();

	return AimRotLS;
}

bool AFPSCharacter::IsEnemyFor(AController* TestPC) const
{
	if (TestPC == Controller || TestPC == NULL)
	{
		return false;
	}

	AFPSPlayerState* TestPlayerState = Cast<AFPSPlayerState>(TestPC->PlayerState);
	AFPSPlayerState* MyPlayerState = Cast<AFPSPlayerState>(PlayerState);

	bool bIsEnemy = true;
	if (GetWorld()->GameState && GetWorld()->GameState->GameModeClass)
	{
		const AFPSGameMode* DefGame = GetWorld()->GameState->GameModeClass->GetDefaultObject<AFPSGameMode>();
		if (DefGame && MyPlayerState && TestPlayerState)
		{
			bIsEnemy = DefGame->CanDealDamage(TestPlayerState, MyPlayerState);
		}
	}

	return bIsEnemy;
}

bool AFPSCharacter::CanReload() const
{
	return true;
}

bool AFPSCharacter::IsTargeting() const
{
	return bIsTargeting;
}

float AFPSCharacter::GetTargetingSpeedModifier() const
{
	return TargetingSpeedModifier;
}

void AFPSCharacter::UpdatePawnMeshes()
{
	const bool bFirstPerson = IsFirstPerson();

	Mesh1P->MeshComponentUpdateFlag = !bFirstPerson ? EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered : EMeshComponentUpdateFlag::AlwaysTickPoseAndRefreshBones;
	Mesh1P->SetOwnerNoSee(!bFirstPerson);

	GetMesh()->MeshComponentUpdateFlag = bFirstPerson ? EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered : EMeshComponentUpdateFlag::AlwaysTickPoseAndRefreshBones;
	GetMesh()->SetOwnerNoSee(bFirstPerson);
}

void AFPSCharacter::UpdateTeamColors(UMaterialInstanceDynamic* UseMID)
{
	if (UseMID)
	{
		AFPSPlayerState* MyPlayerState = Cast<AFPSPlayerState>(PlayerState);
		if (MyPlayerState != NULL)
		{
			float MaterialParam = (float)MyPlayerState->GetTeamNum();
			UseMID->SetScalarParameterValue(TEXT("Team Color Index"), MaterialParam);
		}
	}
}

void AFPSCharacter::UpdateTeamColorsAllMIDs()
{
	for (int32 i = 0; i < MeshMIDs.Num(); ++i)
	{
		UpdateTeamColors(MeshMIDs[i]);
	}
}

bool AFPSCharacter::IsFirstPerson() const
{
	return IsAlive() && Controller && Controller->IsLocalPlayerController();
}

FName AFPSCharacter::GetWeaponAttachPoint() const
{
	return WeaponAttachPoint;
}

USkeletalMeshComponent* AFPSCharacter::GetPawnMesh() const
{
	return IsFirstPerson() ? Mesh1P : GetMesh();
}

USkeletalMeshComponent* AFPSCharacter::GetSpecifcPawnMesh(bool bWantFirstPerson) const
{
	return bWantFirstPerson == true ? Mesh1P : GetMesh();
}

float AFPSCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	AFPSPlayerController* MyPC = Cast<AFPSPlayerController>(Controller);
	if (MyPC && MyPC->HasGodMode())
	{
		return 0.f;
	}

	if (Health <= 0.f)
	{
		return 0.f;
	}

	// Modify based on game rules.
	AFPSGameMode* const Game = GetWorld()->GetAuthGameMode<AFPSGameMode>();
	Damage = Game ? Game->ModifyDamage(Damage, this, DamageEvent, EventInstigator, DamageCauser) : 0.f;

	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.f)
	{
		Health -= ActualDamage;
		if (Health <= 0)
		{
			Die(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
		}
		else
		{
			Hit(ActualDamage, DamageEvent, EventInstigator ? EventInstigator->GetPawn() : NULL, DamageCauser);
		}

		MakeNoise(1.0f, EventInstigator ? EventInstigator->GetPawn() : this);
	}

	return ActualDamage;
}

void AFPSCharacter::Suicide()
{
	KillBy(this);
}

void AFPSCharacter::KillBy(APawn* EventInstigator)
{
	if (!bIsDying)
	{
		AController* Killer = NULL;
		if (EventInstigator != NULL)
		{
			Killer = EventInstigator->Controller;
			LastHitBy = NULL;
		}

		Die(Health, FDamageEvent(UDamageType::StaticClass()), Killer, NULL);
	}
}

bool AFPSCharacter::Die(float KillingDamage, struct FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser)
{
	if (!CanDie(KillingDamage, DamageEvent, Killer, DamageCauser))
	{
		return false;
	}

	Health = FMath::Min(0.0f, Health);

	// if this is an environmental death then refer to the previous killer so that they receive credit (knocked into lava pits, etc)
	UDamageType const* const DamageType = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
	Killer = GetDamageInstigator(Killer, *DamageType);

	AController* const KilledPlayer = (Controller != NULL) ? Controller : Cast<AController>(GetOwner());
	GetWorld()->GetAuthGameMode<AFPSGameMode>()->Killed(Killer, KilledPlayer, this, DamageType);

	DieActual(KillingDamage, DamageEvent, Killer ? Killer->GetPawn() : NULL, DamageCauser);
	return true;
}

bool AFPSCharacter::CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const
{
	if (bIsDying										// already dying
		|| IsPendingKill()								// already destroyed
		|| GetWorld()->GetAuthGameMode() == NULL
		|| GetWorld()->GetAuthGameMode()->GetMatchState() == MatchState::LeavingMap)	// level transition occurring
	{
		return false;
	}

	return true;
}

void AFPSCharacter::FellOutOfWorld(const UDamageType& dmgType)
{
	Die(Health, FDamageEvent(dmgType.GetClass()), NULL, NULL);
}

void AFPSCharacter::DieActual(float KillingDamage, struct FDamageEvent const& DamageEvent, APawn* InstigatingPawn, AActor* DamageCauser)
{
	if (bIsDying)
	{
		return;
	}

	bTearOff = true;
	bIsDying = true;

	if (DeathSound && Mesh1P && Mesh1P->IsVisible())
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}

	// remove all weapons
	DestroyInventory();

	// switch back to 3rd person view
	UpdatePawnMeshes();

	DetachFromControllerPendingDestroy();
	StopAllAnimMontages();

	if (LowHealthWarningPlayer && LowHealthWarningPlayer->IsPlaying())
	{
		LowHealthWarningPlayer->Stop();
	}

	if (RunLoopAC)
	{
		RunLoopAC->Stop();
	}

	if (GetMesh())
	{
		static FName CollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetCollisionProfileName(CollisionProfileName);
	}
	SetActorEnableCollision(true);

	// Death anim
	float DeathAnimDuration = PlayAnimMontage(DeathAnim);

	// Ragdoll
	if (DeathAnimDuration > 0.f)
	{
		// Use a local timer handle as we don't need to store it for later but we don't need to look for something to clear
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AFPSCharacter::EnableRagdollPhysics, FMath::Min(0.1f, DeathAnimDuration), false);
	}
	else
	{
		EnableRagdollPhysics();
	}

	// disable collisions on capsule
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void AFPSCharacter::Hit(float DamageTaken, struct FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser)
{
	if (DamageTaken > 0.f)
	{
		ApplyDamageMomentum(DamageTaken, DamageEvent, PawnInstigator, DamageCauser);
	}

	AFPSPlayerController* MyPC = Cast<AFPSPlayerController>(Controller);
	AGameHUD* MyHUD = MyPC ? Cast<AGameHUD>(MyPC->GetHUD()) : NULL;
	if (MyHUD)
	{
		MyHUD->NotifyWeaponHit(DamageTaken, DamageEvent, PawnInstigator);
	}

	if (PawnInstigator && PawnInstigator != this)
	{
		AFPSPlayerController* InstigatorPC = Cast<AFPSPlayerController>(PawnInstigator->Controller);
		AGameHUD* InstigatorHUD = InstigatorPC ? Cast<AGameHUD>(InstigatorPC->GetHUD()) : NULL;
		if (InstigatorHUD)
		{
			InstigatorHUD->NotifyEnemyHit();
		}
	}
}

void AFPSCharacter::EnableRagdollPhysics()
{
	bool bInRagdoll = false;

	if (IsPendingKill())
	{
		bInRagdoll = false;
	}
	else if (!GetMesh() || !GetMesh()->GetPhysicsAsset())
	{
		bInRagdoll = false;
	}
	else
	{
		// initialize physics/etc
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->WakeAllRigidBodies();
		GetMesh()->bBlendPhysics = true;

		bInRagdoll = true;
	}

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->SetComponentTickEnabled(false);

	if (!bInRagdoll)
	{
		// hide and set short lifespan
		TurnOff();
		SetActorHiddenInGame(true);
		SetLifeSpan(1.0f);
	}
	else
	{
		SetLifeSpan(10.0f);
	}
}


int32 AFPSCharacter::GetMaxHealth() const
{
	return GetClass()->GetDefaultObject<AFPSCharacter>()->Health;
}

bool AFPSCharacter::IsAlive() const
{
	return Health > 0;
}

AFPSWeapon* AFPSCharacter::GetWeapon() const
{
	return WeaponCurrent;
}

AFPSWeapon* AFPSCharacter::FindWeapon(TSubclassOf<AFPSWeapon> WeaponClass)
{
	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		if (Inventory[i] && Inventory[i]->IsA(WeaponClass))
		{
			return Inventory[i];
		}
	}

	return NULL;
}

void AFPSCharacter::SpawnDefaultInventory()
{
	int32 NumWeaponClasses = DefaultInventoryClasses.Num();
	for (int32 i = 0; i < NumWeaponClasses; i++)
	{
		if (DefaultInventoryClasses[i])
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AFPSWeapon* NewWeapon = GetWorld()->SpawnActor<AFPSWeapon>(DefaultInventoryClasses[i], SpawnInfo);
			AddWeapon(NewWeapon);
		}
	}

	// equip first weapon in inventory
	if (Inventory.Num() > 0)
	{
		EquipWeapon(Inventory[0]);
	}
}

void AFPSCharacter::DestroyInventory()
{
	// remove all weapons from inventory and destroy them
	for (int32 i = Inventory.Num() - 1; i >= 0; i--)
	{
		AFPSWeapon* Weapon = Inventory[i];
		if (Weapon)
		{
			RemoveWeapon(Weapon);
			Weapon->Destroy();
		}
	}
}

void AFPSCharacter::AddWeapon(AFPSWeapon* Weapon)
{
	if (Weapon)
	{
		Weapon->OnEnterInventory(this);
		Inventory.AddUnique(Weapon);

		AFPSPlayerController* MyPC = Cast<AFPSPlayerController>(Controller);
		AGameHUD* MyHUD = MyPC ? Cast<AGameHUD>(MyPC->GetHUD()) : NULL;
		if (MyHUD)
		{
			MyHUD->ReceiveInventoryChanged(Inventory);
		}

	}
}

void AFPSCharacter::RemoveWeapon(AFPSWeapon* Weapon)
{
	if (Weapon)
	{
		Weapon->OnLeaveInventory();
		Inventory.RemoveSingle(Weapon);
	}
}

void AFPSCharacter::EquipWeapon(AFPSWeapon* Weapon)
{
	if (Weapon)
	{
		SetCurrentWeapon(Weapon, WeaponCurrent);
	}
}

void AFPSCharacter::UnEquipWeapon(AFPSWeapon* Weapon)
{
	if (Weapon && Weapon == WeaponCurrent)
	{
		SetCurrentWeapon(nullptr, Weapon);
	}
}

void AFPSCharacter::SetCurrentWeapon(AFPSWeapon* NewWeapon, AFPSWeapon* LastWeapon /*= NULL*/)
{
	AFPSWeapon* LocalLastWeapon = NULL;

	if (LastWeapon != NULL)
	{
		LocalLastWeapon = LastWeapon;
	}
	else if (NewWeapon != WeaponCurrent)
	{
		LocalLastWeapon = WeaponCurrent;
	}

	// unequip previous
	if (LocalLastWeapon)
	{
		LocalLastWeapon->OnUnEquip();
	}

	WeaponCurrent = NewWeapon;

	// equip new one
	if (NewWeapon)
	{
		NewWeapon->SetOwningCharacter(this);	// Make sure weapon's MyPawn is pointing back to us. During replication, we can't guarantee APawn::CurrentWeapon will rep after AWeapon::MyPawn!

		NewWeapon->OnEquip(LastWeapon);
	}
}

bool AFPSCharacter::IsRunning() const
{
	if (!GetCharacterMovement())
	{
		return false;
	}

	return (bWantsToRun || bWantsToRunToggled) && !GetVelocity().IsZero() && (GetVelocity().GetSafeNormal2D() | GetActorForwardVector()) > -0.1;
}

float AFPSCharacter::GetRunningSpeedModifier() const
{
	return RunningSpeedModifier;
}

bool AFPSCharacter::CanFire() const
{
	return IsAlive();
}

void AFPSCharacter::SetRunning(bool bNewRunning, bool bToggle)
{
	bWantsToRun = bNewRunning;
	bWantsToRunToggled = bNewRunning && bToggle;

	UpdateRunSounds(bNewRunning);
}

void AFPSCharacter::UpdateRunSounds(bool bNewRunning)
{
	if (bNewRunning)
	{
		if (!RunLoopAC && RunLoopSound)
		{
			RunLoopAC = UGameplayStatics::SpawnSoundAttached(RunLoopSound, GetRootComponent());
			if (RunLoopAC)
			{
				RunLoopAC->bAutoDestroy = false;
			}

		}
		else if (RunLoopAC)
		{
			RunLoopAC->Play();
		}
	}
	else
	{
		if (RunLoopAC)
		{
			RunLoopAC->Stop();
		}

		if (RunStopSound)
		{
			UGameplayStatics::SpawnSoundAttached(RunStopSound, GetRootComponent());
		}
	}
}

void AFPSCharacter::StartWeaponFire()
{
	if (!bWantsToFire)
	{
		bWantsToFire = true;
		if (WeaponCurrent)
		{
			WeaponCurrent->StartFire();
		}
	}
}

void AFPSCharacter::StopWeaponFire()
{
	if (bWantsToFire)
	{
		bWantsToFire = false;
		if (WeaponCurrent)
		{
			WeaponCurrent->StopFire();
		}
	}
}

void AFPSCharacter::SetTargeting(bool bNewTargeting)
{
	bIsTargeting = bNewTargeting;

	if (TargetingSound)
	{
		UGameplayStatics::SpawnSoundAttached(TargetingSound, GetRootComponent());
	}
}

float AFPSCharacter::PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate /*= 1.f*/, FName StartSectionName /*= NAME_None*/)
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (AnimMontage && UseMesh && UseMesh->AnimScriptInstance)
	{
		return UseMesh->AnimScriptInstance->Montage_Play(AnimMontage, InPlayRate);
	}

	return 0.0f;
}

void AFPSCharacter::StopAnimMontage(class UAnimMontage* AnimMontage)
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (AnimMontage && UseMesh && UseMesh->AnimScriptInstance &&
		UseMesh->AnimScriptInstance->Montage_IsPlaying(AnimMontage))
	{
		UseMesh->AnimScriptInstance->Montage_Stop(AnimMontage->BlendOutTime);
	}
}

void AFPSCharacter::StopAllAnimMontages()
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (UseMesh && UseMesh->AnimScriptInstance)
	{
		UseMesh->AnimScriptInstance->Montage_Stop(0.0f);
	}
}

AUsableActor* AFPSCharacter::GetUsableActorInView()
{
	FVector CamLoc;
	FRotator CamRot;

	if (Controller == nullptr)
	{
		return nullptr;
	}

	Controller->GetPlayerViewPoint(CamLoc, CamRot);

	const FVector TraceStart = CamLoc;
	const FVector Direction = CamRot.Vector();
	const FVector TraceEnd = TraceStart + (Direction * MaxUseDistance);

	FCollisionQueryParams TraceParams(TEXT("TraceUsableActor"), true, this);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = false;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams);

	//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 2);
	
	return Cast<AUsableActor>(Hit.GetActor());
}

void AFPSCharacter::TickViewUsable()
{
	if (Controller && Controller->IsLocalController())
	{
		AUsableActor* TheUsableActor = GetUsableActorInView();

		if (FocusedUsableActor != TheUsableActor)
		{
			if (FocusedUsableActor)
			{
				FocusedUsableActor->OnEndFocus();
			}
			bHasNewFocus = true;
		}

		FocusedUsableActor = TheUsableActor;

		if (TheUsableActor)
		{
			if (bHasNewFocus)
			{
				TheUsableActor->OnBeginFocus();
				bHasNewFocus = false;
			}
		}
	}
}