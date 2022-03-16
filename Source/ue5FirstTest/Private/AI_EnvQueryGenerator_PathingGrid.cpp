// Fill out your copyright notice in the Description page of Project Settings.

//#include "EqsTut.h"
#include "AI_EnvQueryGenerator_PathingGrid.h"

void UAI_EnvQueryGenerator_PathingGrid::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	UObject* bindOwner = QueryInstance.Owner.Get();
	
	GridSize.BindData(bindOwner, QueryInstance.QueryID);

	SpaceBetween.BindData(bindOwner, QueryInstance.QueryID);

	OffsetSpace.BindData(bindOwner, QueryInstance.QueryID);

	float sizeValue = GridSize.GetValue();

	float densityValue = SpaceBetween.GetValue();
	float offsetValue = OffsetSpace.GetValue();

	const int32 itemsCount = FPlatformMath::TruncToInt((sizeValue * 2.0f / densityValue) + 1);

	const int32 itemsCountHalf = itemsCount / 2;

	const int32 LeftRangeIndex = itemsCountHalf - FPlatformMath::TruncToInt(offsetValue / densityValue) - 1;
	const int32 RightRangeIndex = itemsCountHalf + FPlatformMath::TruncToInt(offsetValue / densityValue) + 1;
	const int32 OffsetItemsCount = FPlatformMath::TruncToInt((itemsCount * 2.0 / densityValue) + 1);

	TArray<FVector> ContextLocations;
	QueryInstance.PrepareContext(GenerateAround, ContextLocations);

	//the total items count is calculated subtracting the items located into the hole from the total list of items.
	TArray<FNavLocation> GridPoints;

	GridPoints.Reserve(((itemsCount * itemsCount) - (OffsetItemsCount * OffsetItemsCount)) * ContextLocations.Num());
	for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++) {
		for (int32 IndexX = 0; IndexX < itemsCount; ++IndexX)
		{
			for (int32 IndexY = 0; IndexY < itemsCount; ++IndexY)
			{
				// if the item is inside the hole ranges, just skip it.
				if ((IndexY > LeftRangeIndex && IndexY < RightRangeIndex) && (IndexX > LeftRangeIndex && IndexX < RightRangeIndex)) {
					continue;
				}
				// starting from the context location, define the location of the current item 
				// and add it to the gridPoints array.
				else {
					const FNavLocation TestPoint = FNavLocation(ContextLocations[ContextIndex] - FVector(densityValue * (IndexX - itemsCountHalf), densityValue * (IndexY - itemsCountHalf), 0));
					GridPoints.Add(TestPoint);
				}
			}
		}
	}
	ProjectAndFilterNavPoints(GridPoints, QueryInstance);
	StoreNavPoints(GridPoints, QueryInstance);
}

