// Fill out your copyright notice in the Description page of Project Settings.


#include "AdjacencyMap.h"

UAdjacencyMap::UAdjacencyMap()
{
    adjacentVertexCoords = { "", "", "", "", "", "" };
}

void UAdjacencyMap::setDirectionsAdjacency(int direction, FString coord)
{
    adjacentVertexCoords[direction] = coord;
}

void UAdjacencyMap::setAllAdjacencies(TArray<FString> adjacencies)
{
	for (int direction = 0; direction < adjacencies.Num(); direction++)
	{
		setDirectionsAdjacency(direction, adjacencies[direction]);
	}
}

FString UAdjacencyMap::adjacencyString() {
    FString Result;

    Result += FString::Printf(TEXT("North: %s\n"), *adjacentVertexCoords[0]);
    Result += FString::Printf(TEXT("North-East: %s\n"), *adjacentVertexCoords[1]);
    Result += FString::Printf(TEXT("South-East: %s\n"), *adjacentVertexCoords[2]);
    Result += FString::Printf(TEXT("South: %s\n"), *adjacentVertexCoords[3]);
    Result += FString::Printf(TEXT("South-West: %s\n"), *adjacentVertexCoords[4]);
    Result += FString::Printf(TEXT("North-West: %s\n"), *adjacentVertexCoords[5]);

    return Result;
}
