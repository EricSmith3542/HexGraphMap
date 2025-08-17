// Copyright Epic Games, Inc. All Rights Reserved.

#include "HexGraphValidation.h"
#include "HexGraphSettings.h"
#include "HexagonDirection.h"
#include "VertexType.h"

EHexGraphValidationResult UHexGraphValidation::ValidateVertex(const AVertex* Vertex, EVertexType ExpectedType)
{
    // Check if vertex pointer is valid
    if (!IsValid(Vertex))
    {
        UE_LOG(LogHexGraph, Warning, TEXT("ValidateVertex: Null vertex pointer"));
        return EHexGraphValidationResult::NullPointer;
    }

    // Check vertex type if specified
    if (ExpectedType != EVertexType::Vertex && Vertex->type != ExpectedType)
    {
        UE_LOG(LogHexGraph, Warning, TEXT("ValidateVertex: Expected type %d but got %d for vertex at (%d,%d)"), 
               (int32)ExpectedType, (int32)Vertex->type, Vertex->row, Vertex->col);
        return EHexGraphValidationResult::InvalidType;
    }

    // Validate coordinates are within reasonable bounds
    if (!IsValidCoordinate(Vertex->row, Vertex->col))
    {
        UE_LOG(LogHexGraph, Warning, TEXT("ValidateVertex: Invalid coordinates (%d, %d) for vertex"), 
               Vertex->row, Vertex->col);
        return EHexGraphValidationResult::InvalidCoordinate;
    }

    return EHexGraphValidationResult::Valid;
}

bool UHexGraphValidation::IsValidCoordinateString(const FString& CoordinateString)
{
    // Check if string is empty
    if (CoordinateString.IsEmpty())
    {
        return false;
    }

    // Check if string contains the separator ':'
    if (!CoordinateString.Contains(TEXT(":")))
    {
        UE_LOG(LogHexGraph, VeryVerbose, TEXT("IsValidCoordinateString: Missing ':' separator in '%s'"), *CoordinateString);
        return false;
    }

    // Split the string and validate format
    TArray<FString> Parts;
    CoordinateString.ParseIntoArray(Parts, TEXT(":"), true);

    // Should have exactly 2 parts (row and column)
    if (Parts.Num() != 2)
    {
        UE_LOG(LogHexGraph, VeryVerbose, TEXT("IsValidCoordinateString: Expected 2 parts but got %d in '%s'"), 
               Parts.Num(), *CoordinateString);
        return false;
    }

    // Validate that both parts are numeric
    int32 Row, Col;
    if (!FCString::IsNumeric(*Parts[0]) || !FCString::IsNumeric(*Parts[1]))
    {
        UE_LOG(LogHexGraph, VeryVerbose, TEXT("IsValidCoordinateString: Non-numeric parts in '%s'"), *CoordinateString);
        return false;
    }

    // Parse and validate coordinate values
    Row = FCString::Atoi(*Parts[0]);
    Col = FCString::Atoi(*Parts[1]);
    
    return IsValidCoordinate(Row, Col);
}

bool UHexGraphValidation::IsValidCoordinate(int32 Row, int32 Col)
{
    // Get bounds from settings
    int32 MinValue = GetMinCoordinateValue();
    int32 MaxValue = GetMaxCoordinateValue();
    
    // Check bounds to prevent overflow and unreasonable values
    if (Row < MinValue || Row > MaxValue)
    {
        UE_LOG(LogHexGraph, VeryVerbose, TEXT("IsValidCoordinate: Row %d out of bounds [%d, %d]"), 
               Row, MinValue, MaxValue);
        return false;
    }

    if (Col < MinValue || Col > MaxValue)
    {
        UE_LOG(LogHexGraph, VeryVerbose, TEXT("IsValidCoordinate: Col %d out of bounds [%d, %d]"), 
               Col, MinValue, MaxValue);
        return false;
    }

    return true;
}

EHexGraphValidationResult UHexGraphValidation::ValidateAdjacencyMap(const UAdjacencyMap* AdjacencyMap)
{
    // Check if adjacency map pointer is valid
    if (!IsValid(AdjacencyMap))
    {
        UE_LOG(LogHexGraph, Warning, TEXT("ValidateAdjacencyMap: Null adjacency map pointer"));
        return EHexGraphValidationResult::NullPointer;
    }

    // Check if adjacency array has correct size (should be 6 for hexagonal directions)
    if (AdjacencyMap->adjacentVertexCoords.Num() != 6)
    {
        UE_LOG(LogHexGraph, Warning, TEXT("ValidateAdjacencyMap: Expected 6 adjacencies but got %d"), 
               AdjacencyMap->adjacentVertexCoords.Num());
        return EHexGraphValidationResult::InvalidAdjacency;
    }

    // Validate each coordinate string in the adjacency map
    for (int32 i = 0; i < AdjacencyMap->adjacentVertexCoords.Num(); ++i)
    {
        const FString& CoordStr = AdjacencyMap->adjacentVertexCoords[i];
        
        // Empty strings are allowed (no neighbor in that direction)
        if (CoordStr.IsEmpty())
        {
            continue;
        }

        // Validate non-empty coordinate strings
        if (!IsValidCoordinateString(CoordStr))
        {
            UE_LOG(LogHexGraph, Warning, TEXT("ValidateAdjacencyMap: Invalid coordinate string '%s' at index %d"), 
                   *CoordStr, i);
            return EHexGraphValidationResult::InvalidCoordinate;
        }
    }

    return EHexGraphValidationResult::Valid;
}

bool UHexGraphValidation::IsValidDirection(EHexagonDirection Direction)
{
    // Check if direction is within valid enum range (0-5 for hexagonal directions)
    int32 DirectionValue = static_cast<int32>(Direction);
    
    if (DirectionValue < 0 || DirectionValue > 5)
    {
        UE_LOG(LogHexGraph, VeryVerbose, TEXT("IsValidDirection: Direction value %d out of range [0, 5]"), DirectionValue);
        return false;
    }

    return true;
}

bool UHexGraphValidation::ValidateVertexCoordinates(const AVertex* Vertex, int32 ExpectedRow, int32 ExpectedCol)
{
    if (!IsValid(Vertex))
    {
        UE_LOG(LogHexGraph, Warning, TEXT("ValidateVertexCoordinates: Null vertex pointer"));
        return false;
    }

    if (Vertex->row != ExpectedRow || Vertex->col != ExpectedCol)
    {
        UE_LOG(LogHexGraph, Warning, TEXT("ValidateVertexCoordinates: Expected (%d, %d) but got (%d, %d)"), 
               ExpectedRow, ExpectedCol, Vertex->row, Vertex->col);
        return false;
    }

    return true;
}

int32 UHexGraphValidation::GetMaxCoordinateValue()
{
    const UHexGraphSettings* Settings = UHexGraphSettings::GetHexGraphSettings();
    return Settings ? Settings->MaxCoordinateValue : 10000;
}

int32 UHexGraphValidation::GetMinCoordinateValue()
{
    const UHexGraphSettings* Settings = UHexGraphSettings::GetHexGraphSettings();
    return Settings ? Settings->MinCoordinateValue : -10000;
}