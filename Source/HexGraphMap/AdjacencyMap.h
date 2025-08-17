// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "AdjacencyMap.generated.h"

/**
 * 
 */
UCLASS()
class HEXGRAPHMAP_API UAdjacencyMap : public UObject
{
	GENERATED_BODY()

public:
	UAdjacencyMap();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HexGraph")
	TArray<FString> adjacentVertexCoords;

	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	void setDirectionsAdjacency(int direction, FString coord);

	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	void setAllAdjacencies(TArray<FString> adjacencies);

	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	FString adjacencyString();
};
