// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HexGraphMap.h"
#include "Vertex.h"
#include "AdjacencyMap.h"
#include "HexGraphValidation.generated.h"

/**
 * Validation utilities and macros for HexGraph system
 * Provides common validation patterns and error checking functionality
 */

// Validation result enumeration
UENUM(BlueprintType)
enum class EHexGraphValidationResult : uint8
{
    Valid = 0,
    InvalidVertex,
    InvalidCoordinate,
    InvalidAdjacency,
    NullPointer,
    OutOfBounds,
    InvalidType
};

/**
 * Validation macros for common error checking patterns
 */

// Validate pointer is not null
#define HEXGRAPH_VALIDATE_PTR(Ptr, ReturnValue) \
    if (!IsValid(Ptr)) \
    { \
        UE_LOG(LogHexGraph, Error, TEXT("%s: Invalid pointer in %s at line %d"), TEXT(#Ptr), TEXT(__FUNCTION__), __LINE__); \
        return ReturnValue; \
    }

// Validate pointer with custom error message
#define HEXGRAPH_VALIDATE_PTR_MSG(Ptr, ReturnValue, Message) \
    if (!IsValid(Ptr)) \
    { \
        UE_LOG(LogHexGraph, Error, TEXT("%s: %s in %s at line %d"), TEXT(#Ptr), TEXT(Message), TEXT(__FUNCTION__), __LINE__); \
        return ReturnValue; \
    }

// Validate vertex pointer and type
#define HEXGRAPH_VALIDATE_VERTEX(Vertex, ReturnValue) \
    if (!IsValid(Vertex)) \
    { \
        UE_LOG(LogHexGraph, Error, TEXT("Invalid vertex pointer in %s at line %d"), TEXT(__FUNCTION__), __LINE__); \
        return ReturnValue; \
    }

// Validate coordinate string format
#define HEXGRAPH_VALIDATE_COORD_STRING(CoordStr, ReturnValue) \
    if (CoordStr.IsEmpty() || !UHexGraphValidation::IsValidCoordinateString(CoordStr)) \
    { \
        UE_LOG(LogHexGraph, Error, TEXT("Invalid coordinate string '%s' in %s at line %d"), *CoordStr, TEXT(__FUNCTION__), __LINE__); \
        return ReturnValue; \
    }

// Validate array bounds
#define HEXGRAPH_VALIDATE_ARRAY_INDEX(Array, Index, ReturnValue) \
    if (!Array.IsValidIndex(Index)) \
    { \
        UE_LOG(LogHexGraph, Error, TEXT("Array index %d out of bounds (size: %d) in %s at line %d"), Index, Array.Num(), TEXT(__FUNCTION__), __LINE__); \
        return ReturnValue; \
    }

// Validate direction enum value
#define HEXGRAPH_VALIDATE_DIRECTION(Direction, ReturnValue) \
    if (!UHexGraphValidation::IsValidDirection(Direction)) \
    { \
        UE_LOG(LogHexGraph, Error, TEXT("Invalid direction value %d in %s at line %d"), (int32)Direction, TEXT(__FUNCTION__), __LINE__); \
        return ReturnValue; \
    }

/**
 * Static utility class for validation functions
 */
UCLASS()
class HEXGRAPHMAP_API UHexGraphValidation : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Validates a vertex pointer and optionally checks its type
     * @param Vertex The vertex to validate
     * @param ExpectedType Optional expected vertex type to check
     * @return Validation result
     */
    UFUNCTION(BlueprintCallable, Category = "HexGraph|Validation")
    static EHexGraphValidationResult ValidateVertex(const AVertex* Vertex, EVertexType ExpectedType = EVertexType::Vertex);

    /**
     * Validates a coordinate string format (should be "row:col")
     * @param CoordinateString The coordinate string to validate
     * @return True if valid format, false otherwise
     */
    UFUNCTION(BlueprintCallable, Category = "HexGraph|Validation")
    static bool IsValidCoordinateString(const FString& CoordinateString);

    /**
     * Validates coordinate values are within reasonable bounds
     * @param Row The row coordinate
     * @param Col The column coordinate
     * @return True if coordinates are valid, false otherwise
     */
    UFUNCTION(BlueprintCallable, Category = "HexGraph|Validation")
    static bool IsValidCoordinate(int32 Row, int32 Col);

    /**
     * Validates an adjacency map
     * @param AdjacencyMap The adjacency map to validate
     * @return Validation result
     */
    UFUNCTION(BlueprintCallable, Category = "HexGraph|Validation")
    static EHexGraphValidationResult ValidateAdjacencyMap(const UAdjacencyMap* AdjacencyMap);

    /**
     * Validates a hexagon direction enum value
     * @param Direction The direction to validate
     * @return True if valid direction, false otherwise
     */
    UFUNCTION(BlueprintCallable, Category = "HexGraph|Validation")
    static bool IsValidDirection(EHexagonDirection Direction);

    /**
     * Validates vertex coordinates match expected position
     * @param Vertex The vertex to check
     * @param ExpectedRow Expected row coordinate
     * @param ExpectedCol Expected column coordinate
     * @return True if coordinates match, false otherwise
     */
    UFUNCTION(BlueprintCallable, Category = "HexGraph|Validation")
    static bool ValidateVertexCoordinates(const AVertex* Vertex, int32 ExpectedRow, int32 ExpectedCol);

private:
    // Get coordinate bounds from settings instead of hard-coded values
    static int32 GetMaxCoordinateValue();
    static int32 GetMinCoordinateValue();
};