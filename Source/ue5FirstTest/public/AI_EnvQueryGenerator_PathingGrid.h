// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_PathingGrid.h"
#include "AI_EnvQueryGenerator_PathingGrid.generated.h"

/**
 * 
 */
UCLASS()
class UE5FIRSTTEST_API UAI_EnvQueryGenerator_PathingGrid : public UEnvQueryGenerator_PathingGrid
{
	GENERATED_BODY()

	/** Generates items in a cone and places them in the environemtn */
	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;

};
