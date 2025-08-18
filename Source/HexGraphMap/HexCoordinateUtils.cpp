// Copyright Epic Games, Inc. All Rights Reserved.

#include "HexCoordinateUtils.h"
#include "HexGraphSettings.h"
#include "HexGraphMap.h"
#include "Math/UnrealMathUtility.h"

FHexCoordinate FHexCoordinateUtils::GetCoordinateInDirection(const FHexCoordinate& Coordinate, EHexagonDirection Direction)
{
	return ApplyDirectionOffset(Coordinate, Direction);
}

TArray<FHexCoordinate> FHexCoordinateUtils::GetAdjacentCoordinates(const FHexCoordinate& Coordinate)
{
	TArray<FHexCoordinate> AdjacentCoords;
	AdjacentCoords.Reserve(6);

	// Get all 6 adjacent coordinates
	for (int32 i = 0; i < 6; ++i)
	{
		EHexagonDirection Direction = static_cast<EHexagonDirection>(i);
		AdjacentCoords.Add(GetCoordinateInDirection(Coordinate, Direction));
	}

	return AdjacentCoords;
}

int32 FHexCoordinateUtils::CalculateDistance(const FHexCoordinate& From, const FHexCoordinate& To)
{
	// Hexagonal distance calculation using axial coordinates
	// Convert to cube coordinates for easier distance calculation
	int32 x1 = From.Col;
	int32 z1 = From.Row - (From.Col - (From.Col & 1)) / 2;
	int32 y1 = -x1 - z1;

	int32 x2 = To.Col;
	int32 z2 = To.Row - (To.Col - (To.Col & 1)) / 2;
	int32 y2 = -x2 - z2;

	return (FMath::Abs(x1 - x2) + FMath::Abs(y1 - y2) + FMath::Abs(z1 - z2)) / 2;
}

EHexagonDirection FHexCoordinateUtils::GetDirectionTo(const FHexCoordinate& From, const FHexCoordinate& To)
{
	if (From == To)
	{
		return EHexagonDirection::North; // Default direction when coordinates are equal
	}

	// Calculate the vector difference
	FHexCoordinate Diff = To - From;
	
	// Find the direction with the largest component
	// This is a simplified approach - for more complex pathfinding, 
	// a proper hex grid direction calculation would be needed
	
	TArray<FHexCoordinate> Directions;
	for (int32 i = 0; i < 6; ++i)
	{
		EHexagonDirection Direction = static_cast<EHexagonDirection>(i);
		FHexCoordinate DirectionCoord = GetCoordinateInDirection(From, Direction);
		Directions.Add(DirectionCoord - From);
	}

	// Find the direction that best matches our target difference
	int32 BestDirection = 0;
	int32 BestScore = INT32_MAX;
	
	for (int32 i = 0; i < 6; ++i)
	{
		FHexCoordinate TestTarget = From + Directions[i];
		int32 Distance = CalculateDistance(TestTarget, To);
		
		if (Distance < BestScore)
		{
			BestScore = Distance;
			BestDirection = i;
		}
	}

	return static_cast<EHexagonDirection>(BestDirection);
}

EHexagonDirection FHexCoordinateUtils::GetOppositeDirection(EHexagonDirection Direction)
{
	int32 DirectionInt = static_cast<int32>(Direction);
	int32 OppositeInt = (DirectionInt + 3) % 6;
	return static_cast<EHexagonDirection>(OppositeInt);
}

FVector FHexCoordinateUtils::CoordinateToWorldPosition(const FHexCoordinate& Coordinate, float MeshLength, float VertexSpacing)
{
	// Calculate the spacing between hex centers
	float TotalSpacing = MeshLength + VertexSpacing;
	
	// Get the unit vector for northeast direction to calculate column offset
	FVector2D NEVector = GetUnitVectorForDirection(EHexagonDirection::Northeast);
	
	// Calculate column offset (Y direction)
	float ColOffset = ((NEVector * TotalSpacing).Y) * Coordinate.Col;
	
	// Calculate row offset (X direction)
	// Odd columns are offset by negative half a row spacing (to match original system)
	float RowOffset = TotalSpacing * Coordinate.Row;
	if (FMath::Abs(Coordinate.Col % 2) == 1)
	{
		RowOffset -= TotalSpacing * 0.5f;
	}

	return FVector(RowOffset, ColOffset, 0.0f);
}

FHexCoordinate FHexCoordinateUtils::WorldPositionToCoordinate(const FVector& WorldPosition, float MeshLength, float VertexSpacing)
{
	// This is a simplified version of the algorithm from HexGraph.cpp
	float TotalSpacing = MeshLength + VertexSpacing;
	float RowHalfStep = TotalSpacing / 2.0f;
	
	FVector2D NEVector = GetUnitVectorForDirection(EHexagonDirection::Northeast);
	float ColHalfStep = ((NEVector * TotalSpacing).Y) / 2.0f;

	// Calculate column
	int32 ColHalfSteps;
	if (WorldPosition.Y >= 0)
	{
		ColHalfSteps = FMath::CeilToInt(WorldPosition.Y / ColHalfStep);
	}
	else
	{
		ColHalfSteps = FMath::FloorToInt(WorldPosition.Y / ColHalfStep);
	}
	int32 Col = ColHalfSteps / 2;

	// Calculate row with column offset
	float RowDistance = WorldPosition.X;
	if (FMath::Abs(Col % 2) == 1)
	{
		RowDistance += RowHalfStep;
	}

	int32 RowHalfSteps;
	if (WorldPosition.X >= 0)
	{
		RowHalfSteps = FMath::CeilToInt(RowDistance / RowHalfStep);
	}
	else
	{
		RowHalfSteps = FMath::FloorToInt(RowDistance / RowHalfStep);
	}
	int32 Row = RowHalfSteps / 2;

	return FHexCoordinate(Row, Col);
}

FVector2D FHexCoordinateUtils::GetUnitVectorForDirection(EHexagonDirection Direction)
{
	int32 DirectionInt = static_cast<int32>(Direction);
	float Angle = DirectionInt * 60.0f * PI / 180.0f; // Convert to radians
	return FVector2D(FMath::Cos(Angle), FMath::Sin(Angle));
}

bool FHexCoordinateUtils::IsValidCoordinate(const FHexCoordinate& Coordinate, int32 MaxCoordinateValue, int32 MinCoordinateValue)
{
	return Coordinate.Row >= MinCoordinateValue && Coordinate.Row <= MaxCoordinateValue &&
		   Coordinate.Col >= MinCoordinateValue && Coordinate.Col <= MaxCoordinateValue;
}

TArray<FHexCoordinate> FHexCoordinateUtils::GetCoordinatesInRadius(const FHexCoordinate& Center, int32 Radius)
{
	TArray<FHexCoordinate> Coordinates;
	
	if (Radius < 0)
	{
		return Coordinates;
	}

	// Add center coordinate
	if (Radius >= 0)
	{
		Coordinates.Add(Center);
	}

	// Add coordinates in rings around the center
	for (int32 Ring = 1; Ring <= Radius; ++Ring)
	{
		// Start at the "North" position of this ring
		FHexCoordinate Current = Center;
		
		// Move to starting position of this ring
		for (int32 i = 0; i < Ring; ++i)
		{
			Current = GetCoordinateInDirection(Current, EHexagonDirection::North);
		}

		// Walk around the ring
		for (int32 Side = 0; Side < 6; ++Side)
		{
			EHexagonDirection WalkDirection = static_cast<EHexagonDirection>((Side + 2) % 6); // Start with Southeast
			
			for (int32 Step = 0; Step < Ring; ++Step)
			{
				Coordinates.Add(Current);
				Current = GetCoordinateInDirection(Current, WalkDirection);
			}
		}
	}

	return Coordinates;
}

TArray<FHexCoordinate> FHexCoordinateUtils::GetPathBetween(const FHexCoordinate& From, const FHexCoordinate& To)
{
	TArray<FHexCoordinate> Path;
	
	if (From == To)
	{
		Path.Add(From);
		return Path;
	}

	// Simple straight-line path approximation
	// For more sophisticated pathfinding, implement A* or similar
	FHexCoordinate Current = From;
	Path.Add(Current);

	int32 MaxSteps = CalculateDistance(From, To) * 2; // Safety limit
	int32 StepCount = 0;

	while (Current != To && StepCount < MaxSteps)
	{
		EHexagonDirection NextDirection = GetDirectionTo(Current, To);
		Current = GetCoordinateInDirection(Current, NextDirection);
		Path.Add(Current);
		++StepCount;
	}

	return Path;
}

bool FHexCoordinateUtils::AreAdjacent(const FHexCoordinate& First, const FHexCoordinate& Second)
{
	return CalculateDistance(First, Second) == 1;
}

FHexCoordinate FHexCoordinateUtils::ApplyDirectionOffset(const FHexCoordinate& Coordinate, EHexagonDirection Direction)
{
	// This matches the logic from HexGraph.cpp GetCoordInDirection function
	int32 Row = Coordinate.Row;
	int32 Col = Coordinate.Col;

	switch (Direction)
	{
	case EHexagonDirection::North:
		Row += 1;
		break;
	case EHexagonDirection::Northeast:
		if (Col % 2 == 0) {
			Row += 1;
			Col += 1;
		}
		else {
			Col += 1;
		}
		break;
	case EHexagonDirection::Southeast:
		if (Col % 2 != 0) {
			Row -= 1;
			Col += 1;
		}
		else {
			Col += 1;
		}
		break;
	case EHexagonDirection::South:
		Row -= 1;
		break;
	case EHexagonDirection::Southwest:
		if (Col % 2 != 0) {
			Row -= 1;
			Col -= 1;
		}
		else {
			Col -= 1;
		}
		break;
	case EHexagonDirection::Northwest:
		if (Col % 2 == 0) {
			Row += 1;
			Col -= 1;
		}
		else {
			Col -= 1;
		}
		break;
	default:
		break;
	}

	return FHexCoordinate(Row, Col);
}