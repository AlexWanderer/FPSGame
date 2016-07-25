#pragma once

#include "FPSTypes.generated.h"

UENUM(BlueprintType)
enum class EShooterMatchState
{
	Warmup,
	Playing,
	Won,
	Lost,
}; 


USTRUCT()
struct FDecalData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Decal)
	UMaterial* DecalMaterial;

	UPROPERTY(EditDefaultsOnly, Category = Decal)
	float DecalSize;

	UPROPERTY(EditDefaultsOnly, Category = Decal)
	float LifeSpan;

	FDecalData()
		: DecalSize(256.f)
		, LifeSpan(10.f)
	{
	}
};