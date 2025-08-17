// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HexagonDirection.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EHexagonDirection : uint8
{
    North = 0 UMETA(DisplayName = "North"),
    Northeast = 1 UMETA(DisplayName = "Northeast"),
    Southeast = 2 UMETA(DisplayName = "Southeast"),
    South = 3 UMETA(DisplayName = "South"),
    Southwest = 4 UMETA(DisplayName = "Southwest"),
    Northwest = 5 UMETA(DisplayName = "Northwest"),
};