// Copyright Epic Games, Inc. All Rights Reserved.

#include "HexCoordinate.h"
#include "HexGraphValidation.h"
#include "HexGraphMap.h"

FHexCoordinate FHexCoordinate::FromString(const FString& CoordString)
{
	// Validate coordinate string format
	if (!UHexGraphValidation::IsValidCoordinateString(CoordString))
	{
		UE_LOG(LogHexGraph, Error, TEXT("FHexCoordinate::FromString: Invalid coordinate string format '%s'"), *CoordString);
		return FHexCoordinate(0, 0);
	}

	// Parse the coordinate string
	TArray<FString> CoordParts;
	CoordString.ParseIntoArray(CoordParts, TEXT(":"), true);

	// Validate we have exactly 2 parts
	if (CoordParts.Num() != 2)
	{
		UE_LOG(LogHexGraph, Error, TEXT("FHexCoordinate::FromString: Expected 2 parts separated by ':', got %d parts in '%s'"), CoordParts.Num(), *CoordString);
		return FHexCoordinate(0, 0);
	}

	// Convert strings to integers
	int32 ParsedRow = FCString::Atoi(*CoordParts[0]);
	int32 ParsedCol = FCString::Atoi(*CoordParts[1]);

	// Validate the resulting coordinates
	if (!UHexGraphValidation::IsValidCoordinate(ParsedRow, ParsedCol))
	{
		UE_LOG(LogHexGraph, Error, TEXT("FHexCoordinate::FromString: Parsed coordinates (%d, %d) are out of valid range"), ParsedRow, ParsedCol);
		return FHexCoordinate(0, 0);
	}

	return FHexCoordinate(ParsedRow, ParsedCol);
}

FString FHexCoordinate::ToString() const
{
	return FString::Printf(TEXT("%d:%d"), Row, Col);
}

bool FHexCoordinate::IsValid() const
{
	return UHexGraphValidation::IsValidCoordinate(Row, Col);
}

uint32 FHexCoordinate::GetHashValue() const
{
	// Use a simple hash combining Row and Col
	// This should provide good distribution for typical use cases
	return HashCombine(::GetTypeHash(Row), ::GetTypeHash(Col));
}