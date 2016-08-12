// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "FPSPlayerController.h"
#include "Player/FPSPlayerCameraManager.h"
#include "UI/GameHUD.h"
#include "Player/FPSCharacter.h"
#include "Player/FPSPlayerState.h"
#include "Weapon/FPSWeapon.h"

AFPSPlayerController::AFPSPlayerController():Super()
{
	PlayerCameraManagerClass = AFPSPlayerCameraManager::StaticClass();
	LastDeathLocation = FVector::ZeroVector;
	bAllowGameActions = true;
}

void AFPSPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AFPSPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

AGameHUD* AFPSPlayerController::GetGameHUD() const
{
	return Cast<AGameHUD>(GetHUD());
}

bool AFPSPlayerController::FindDeathCameraSpot(FVector& CameraLocation, FRotator& CameraRotation)
{
	const FVector PawnLocation = GetPawn()->GetActorLocation();
	FRotator ViewDir = GetControlRotation();
	ViewDir.Pitch = -45.0f;

	const float YawOffsets[] = { 0.0f, -180.0f, 90.0f, -90.0f, 45.0f, -45.0f, 135.0f, -135.0f };
	const float CameraOffset = 600.0f;
	FCollisionQueryParams TraceParams(TEXT("DeathCamera"), true, GetPawn());

	FHitResult HitResult;
	for (int32 i = 0; i < ARRAY_COUNT(YawOffsets); i++)
	{
		FRotator CameraDir = ViewDir;
		CameraDir.Yaw += YawOffsets[i];
		CameraDir.Normalize();

		const FVector TestLocation = PawnLocation - CameraDir.Vector() * CameraOffset;

		const bool bBlocked = GetWorld()->LineTraceSingleByChannel(HitResult, PawnLocation, TestLocation, ECC_Camera, TraceParams);

		if (!bBlocked)
		{
			CameraLocation = TestLocation;
			CameraRotation = CameraDir;
			return true;
		}
	}

	return false;
}

bool AFPSPlayerController::IsGameInputAllowed() const
{
	return bAllowGameActions && !bCinematicMode;
}

void AFPSPlayerController::SetCinematicMode(bool bInCinematicMode, bool bHidePlayer, bool bAffectsHUD, bool bAffectsMovement, bool bAffectsTurning)
{
	Super::SetCinematicMode(bInCinematicMode, bHidePlayer, bAffectsHUD, bAffectsMovement, bAffectsTurning);

	// If we have a pawn we need to determine if we should show/hide the weapon
	AFPSCharacter* MyPawn = Cast<AFPSCharacter>(GetPawn());
	AFPSWeapon* MyWeapon = MyPawn ? MyPawn->GetWeapon() : NULL;
	if (MyWeapon)
	{
		if (bInCinematicMode && bHidePlayer)
		{
			MyWeapon->SetActorHiddenInGame(true);
		}
		else if (!bCinematicMode)
		{
			MyWeapon->SetActorHiddenInGame(false);
		}
	}
}

bool AFPSPlayerController::IsMoveInputIgnored() const
{
	if (IsInState(NAME_Spectating))
	{
		return false;
	}
	else
	{
		return Super::IsMoveInputIgnored();
	}
}

bool AFPSPlayerController::IsLookInputIgnored() const
{
	if (IsInState(NAME_Spectating))
	{
		return false;
	}
	else
	{
		return Super::IsLookInputIgnored();
	}
}

void AFPSPlayerController::GameHasEnded(class AActor* EndGameFocus /* = NULL */, bool bIsWinner /* = false */)
{
	Super::GameHasEnded(EndGameFocus, bIsWinner);
	// Disable controls now the game has ended
	SetIgnoreMoveInput(true);

	bAllowGameActions = false;

	// Make sure that we still have valid view target
	SetViewTarget(GetPawn());

	AGameHUD* ShooterHUD = GetGameHUD();
	if (ShooterHUD)
	{
		ShooterHUD->SetMatchState(bIsWinner ? EShooterMatchState::Won : EShooterMatchState::Lost);
	}
}

void AFPSPlayerController::FailedToSpawnPawn()
{
	if (StateName == NAME_Inactive)
	{
		BeginInactiveState();
	}
	Super::FailedToSpawnPawn();
}

void AFPSPlayerController::PawnPendingDestroy(APawn* Pawn)
{
	LastDeathLocation = Pawn->GetActorLocation();
	FVector CameraLocation = LastDeathLocation + FVector(0, 0, 300.0f);
	FRotator CameraRotation(-90.0f, 0.0f, 0.0f);
	FindDeathCameraSpot(CameraLocation, CameraRotation);

	Super::PawnPendingDestroy(Pawn);
}

bool AFPSPlayerController::HasInfiniteClip() const
{
	return false;
}

bool AFPSPlayerController::HasInfiniteAmmo() const
{
	return false;
}

bool AFPSPlayerController::HasHealthRegen() const
{
	return false;
}

bool AFPSPlayerController::HasGodMode() const
{
	return false;
}
