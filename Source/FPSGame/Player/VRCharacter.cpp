// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "VRCharacter.h"


// Sets default values
AVRCharacter::AVRCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AVRCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	check(InputComponent);
	InputComponent->BindAction("LT", IE_Pressed, this, &AVRCharacter::OnLT);
	InputComponent->BindAction("RT", IE_Released, this, &AVRCharacter::OnRT);
}

void AVRCharacter::OnLT()
{
	UE_LOG(LogTemp, Log, TEXT("TempL"));
	FTransform t = FTransform();
	t.SetLocation(GetActorLocation());
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	GetWorld()->SpawnActor<AActor>(Temp, t, SpawnInfo);
}

void AVRCharacter::OnRT()
{

	UE_LOG(LogTemp, Log, TEXT("TempR"));

	FTransform t = FTransform();
	t.SetLocation(GetActorLocation());
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GetWorld()->SpawnActor<AActor>(Temp, t, SpawnInfo);
}

