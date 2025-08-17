// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "VertexType.generated.h"

/**
 *
 */
UENUM(BlueprintType)
enum class EVertexType : uint8
{
    Vertex = 0 UMETA(DisplayName = "Vertex"),
    Graph = 1 UMETA(DisplayName = "Graph"),
    PlaceHolder = 2 UMETA(DisplayName = "PlaceHolder"),
    Preview = 3 UMETA(DisplayName = "Preview"),
};
