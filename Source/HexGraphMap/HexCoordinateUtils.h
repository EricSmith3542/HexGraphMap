// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HexCoordinate.h"
#include "HexagonDirection.h"
#include "Math/Vector.h"
#include "Math/Vector2D.h"

/**
 * Static utility functions for hexagonal coordinate operations
 * Provides direction-based movement, distance calculation, and validation
 */
class HEXGRAPHMAP_API FHexCoordinateUtils
{
public:
	/**
	 * Get coordinate in a specific direction from a given coordinate
	 * @param Coordinate Starting coordinate
	 * @param Direction Direction to move in
	 * @return New coordinate in the specified direction
	 */
	static FHexCoordinate GetCoordinateInDirection(const FHexCoordinate& Coordinate, EHexagonDirection Direction);

	/**
	 * Get all adjacent coordinates to a given coordinate
	 * @param Coordinate Center coordinate
	 * @return Array of 6 adjacent coordinates
	 */
	static TArray<FHexCoordinate> GetAdjacentCoordinates(const FHexCoordinate& Coordinate);

	/**
	 * Calculate the distance between two coordinates
	 * @param From Starting coordinate
	 * @param To Ending coordinate
	 * @return Distance in hexagonal grid units
	 */
	static int32 CalculateDistance(const FHexCoordinate& From, const FHexCoordinate& To);

	/**
	 * Calculate the direction from one coordinate to another
	 * @param From Starting coordinate
	 * @param To Target coordinate
	 * @return Direction enum, or North if coordinates are equal
	 */
	static EHexagonDirection GetDirectionTo(const FHexCoordinate& From, const FHexCoordinate& To);

	/**
	 * Get the opposite direction
	 * @param Direction Input direction
	 * @return Opposite direction
	 */
	static EHexagonDirection GetOppositeDirection(EHexagonDirection Direction);

	/**
	 * Convert coordinate to world position based on mesh length and spacing
	 * @param Coordinate Hex coordinate
	 * @param MeshLength Size of each hex cell
	 * @param VertexSpacing Additional spacing between vertices
	 * @return World position (FVector with Z=0)
	 */
	static FVector CoordinateToWorldPosition(const FHexCoordinate& Coordinate, float MeshLength, float VertexSpacing);

	/**
	 * Convert world position to nearest hex coordinate
	 * @param WorldPosition World position (typically ignoring Z)
	 * @param MeshLength Size of each hex cell
	 * @param VertexSpacing Additional spacing between vertices
	 * @return Nearest hex coordinate
	 */
	static FHexCoordinate WorldPositionToCoordinate(const FVector& WorldPosition, float MeshLength, float VertexSpacing);

	/**
	 * Get unit vector for a specific hex direction (in 2D space)
	 * @param Direction Hexagonal direction
	 * @return Unit vector pointing in that direction
	 */
	static FVector2D GetUnitVectorForDirection(EHexagonDirection Direction);

	/**
	 * Validate that a coordinate is within reasonable bounds
	 * @param Coordinate Coordinate to validate
	 * @param MaxCoordinateValue Maximum allowed coordinate value
	 * @param MinCoordinateValue Minimum allowed coordinate value
	 * @return True if coordinate is valid
	 */
	static bool IsValidCoordinate(const FHexCoordinate& Coordinate, int32 MaxCoordinateValue = 10000, int32 MinCoordinateValue = -10000);

	/**
	 * Get coordinates within a specific radius of a center coordinate
	 * @param Center Center coordinate
	 * @param Radius Radius in hex grid units
	 * @return Array of coordinates within radius
	 */
	static TArray<FHexCoordinate> GetCoordinatesInRadius(const FHexCoordinate& Center, int32 Radius);

	/**
	 * Get a path between two coordinates (simple straight line)
	 * @param From Starting coordinate
	 * @param To Ending coordinate
	 * @return Array of coordinates forming a path
	 */
	static TArray<FHexCoordinate> GetPathBetween(const FHexCoordinate& From, const FHexCoordinate& To);

	/**
	 * Check if two coordinates are adjacent (within 1 step)
	 * @param First First coordinate
	 * @param Second Second coordinate
	 * @return True if coordinates are adjacent
	 */
	static bool AreAdjacent(const FHexCoordinate& First, const FHexCoordinate& Second);

private:
	// Private helper functions
	static FHexCoordinate ApplyDirectionOffset(const FHexCoordinate& Coordinate, EHexagonDirection Direction);
};