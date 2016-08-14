// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ArrowComponent.h"
#include "RoomGateComponent.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class URoomGateComponent : public UArrowComponent
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RoomGateComponent)
	bool bHaveConnection = false;
	
	
};
