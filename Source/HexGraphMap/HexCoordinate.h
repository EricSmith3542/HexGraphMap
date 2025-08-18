// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "HexCoordinate.generated.h"

/**
 * Represents a coordinate in a hexagonal grid system
 * Uses row/column system where even columns are offset upward
 */
USTRUCT(BlueprintType)
struct HEXGRAPHMAP_API FHexCoordinate
{
	GENERATED_BODY()

public:
	/** Row coordinate in the hexagonal grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Coordinate")
	int32 Row = 0;

	/** Column coordinate in the hexagonal grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Coordinate")
	int32 Col = 0;

	/** Default constructor */
	FHexCoordinate() = default;

	/** Constructor with row and column */
	FHexCoordinate(int32 InRow, int32 InCol)
		: Row(InRow), Col(InCol)
	{
	}

	/** Create coordinate from row and column */
	static FHexCoordinate FromRowCol(int32 InRow, int32 InCol)
	{
		return FHexCoordinate(InRow, InCol);
	}

	/** Create coordinate from string representation (e.g., "5:3") */
	static FHexCoordinate FromString(const FString& CoordString);

	/** Convert coordinate to string representation */
	FString ToString() const;

	/** Check if this coordinate is valid (within reasonable bounds) */
	bool IsValid() const;

	/** Get the hash value for this coordinate (for use in TMap) */
	uint32 GetHashValue() const;

	// Operators
	bool operator==(const FHexCoordinate& Other) const
	{
		return Row == Other.Row && Col == Other.Col;
	}

	bool operator!=(const FHexCoordinate& Other) const
	{
		return !(*this == Other);
	}

	bool operator<(const FHexCoordinate& Other) const
	{
		if (Row != Other.Row)
		{
			return Row < Other.Row;
		}
		return Col < Other.Col;
	}

	// Arithmetic operators for coordinate manipulation
	FHexCoordinate operator+(const FHexCoordinate& Other) const
	{
		return FHexCoordinate(Row + Other.Row, Col + Other.Col);
	}

	FHexCoordinate operator-(const FHexCoordinate& Other) const
	{
		return FHexCoordinate(Row - Other.Row, Col - Other.Col);
	}

	FHexCoordinate& operator+=(const FHexCoordinate& Other)
	{
		Row += Other.Row;
		Col += Other.Col;
		return *this;
	}

	FHexCoordinate& operator-=(const FHexCoordinate& Other)
	{
		Row -= Other.Row;
		Col -= Other.Col;
		return *this;
	}
};

// Hash function for TMap support
FORCEINLINE uint32 GetTypeHash(const FHexCoordinate& Coordinate)
{
	return Coordinate.GetHashValue();
}

// String conversion for debugging
FORCEINLINE FString LexToString(const FHexCoordinate& Coordinate)
{
	return Coordinate.ToString();
}