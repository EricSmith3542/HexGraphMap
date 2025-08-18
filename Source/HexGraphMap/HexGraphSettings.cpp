// Copyright Epic Games, Inc. All Rights Reserved.

#include "HexGraphSettings.h"
#include "HexGraphMap.h"

#if WITH_EDITOR
#include "Engine/World.h"
#include "UObject/UObjectIterator.h"
#include "HexGraph.h"
#endif

UHexGraphSettings::UHexGraphSettings()
{
	// Set category and section names for the settings panel
	CategoryName = TEXT("Plugins");
	SectionName = TEXT("HexGraphSettings");
	
	// Initialize default values (already set in header, but explicit here for clarity)
	DefaultVertexSpacing = 100.0f;
	MaxFillDepth = 5;
	DefaultMeshLength = 100.0f;
	DefaultPanSpeed = 15.0f;
	DefaultZoomPercent = 100.0f;
	ExpectedMaxVertices = 10000;
	ExpectedMaxAdjacencyMaps = 10000;
	bAutoCleanupEnabled = true;
	CleanupIntervalSeconds = 10.0f;
	MaxCoordinateValue = 10000;
	MinCoordinateValue = -10000;
	bStrictValidationEnabled = true;
	bUseMouseFollower = false;
	bEnableVerboseLogging = false;
	bShowValidationWarnings = true;
	bEnableObjectPooling = false;
	ObjectPoolInitialSize = 100;

	ValidateSettings();
}

const UHexGraphSettings* UHexGraphSettings::GetHexGraphSettings()
{
	return GetDefault<UHexGraphSettings>();
}

UHexGraphSettings* UHexGraphSettings::GetMutableHexGraphSettings()
{
	return GetMutableDefault<UHexGraphSettings>();
}

FName UHexGraphSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}

#if WITH_EDITOR
void UHexGraphSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	// Validate settings when changed in editor
	ValidateSettings();
	
	// Log the change for debugging
	if (PropertyChangedEvent.Property)
	{
		UE_LOG(LogHexGraph, Log, TEXT("HexGraphSettings: Property '%s' changed"), *PropertyChangedEvent.Property->GetName());
	}
	
	// Notify all HexGraph instances to refresh their settings
	RefreshAllHexGraphInstances();
}

void UHexGraphSettings::RefreshAllHexGraphInstances()
{
	UE_LOG(LogHexGraph, Log, TEXT("HexGraphSettings: Refreshing all HexGraph instances with new settings"));
	
	int32 RefreshedInstances = 0;
	
	// Iterate through all HexGraph instances in all worlds
	for (TObjectIterator<AHexGraph> HexGraphIterator; HexGraphIterator; ++HexGraphIterator)
	{
		AHexGraph* HexGraphInstance = *HexGraphIterator;
		if (IsValid(HexGraphInstance))
		{
			// Check if the instance is in a valid world (not being destroyed)
			UWorld* World = HexGraphInstance->GetWorld();
			if (IsValid(World) && !World->bIsTearingDown)
			{
				HexGraphInstance->RefreshSettingsValues();
				RefreshedInstances++;
				UE_LOG(LogHexGraph, VeryVerbose, TEXT("HexGraphSettings: Refreshed settings for HexGraph instance '%s'"), 
					   *HexGraphInstance->GetName());
			}
		}
	}
	
	UE_LOG(LogHexGraph, Log, TEXT("HexGraphSettings: Successfully refreshed %d HexGraph instances"), RefreshedInstances);
}
#endif

void UHexGraphSettings::ValidateSettings()
{
	// Ensure vertex spacing is positive
	if (DefaultVertexSpacing <= 0.0f)
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexGraphSettings: DefaultVertexSpacing must be positive, resetting to 100.0"));
		DefaultVertexSpacing = 100.0f;
	}

	// Ensure max fill depth is reasonable
	if (MaxFillDepth < 1)
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexGraphSettings: MaxFillDepth must be at least 1, resetting to 5"));
		MaxFillDepth = 5;
	}
	else if (MaxFillDepth > 100)
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexGraphSettings: MaxFillDepth too high (%d), clamping to 100"), MaxFillDepth);
		MaxFillDepth = 100;
	}

	// Ensure mesh length is positive
	if (DefaultMeshLength <= 0.0f)
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexGraphSettings: DefaultMeshLength must be positive, resetting to 100.0"));
		DefaultMeshLength = 100.0f;
	}

	// Ensure pan speed is positive
	if (DefaultPanSpeed <= 0.0f)
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexGraphSettings: DefaultPanSpeed must be positive, resetting to 15.0"));
		DefaultPanSpeed = 15.0f;
	}

	// Ensure zoom percent is reasonable
	if (DefaultZoomPercent < 10.0f)
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexGraphSettings: DefaultZoomPercent too low, clamping to 10.0"));
		DefaultZoomPercent = 10.0f;
	}
	else if (DefaultZoomPercent > 1000.0f)
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexGraphSettings: DefaultZoomPercent too high, clamping to 1000.0"));
		DefaultZoomPercent = 1000.0f;
	}

	// Validate memory management settings
	if (ExpectedMaxVertices < 100)
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexGraphSettings: ExpectedMaxVertices too low, clamping to 100"));
		ExpectedMaxVertices = 100;
	}

	if (ExpectedMaxAdjacencyMaps < 100)
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexGraphSettings: ExpectedMaxAdjacencyMaps too low, clamping to 100"));
		ExpectedMaxAdjacencyMaps = 100;
	}

	if (CleanupIntervalSeconds < 1.0f)
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexGraphSettings: CleanupIntervalSeconds too low, clamping to 1.0"));
		CleanupIntervalSeconds = 1.0f;
	}

	// Validate coordinate bounds
	if (MinCoordinateValue >= MaxCoordinateValue)
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexGraphSettings: MinCoordinateValue must be less than MaxCoordinateValue, resetting to defaults"));
		MinCoordinateValue = -10000;
		MaxCoordinateValue = 10000;
	}

	// Validate object pool settings
	if (ObjectPoolInitialSize < 10)
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexGraphSettings: ObjectPoolInitialSize too low, clamping to 10"));
		ObjectPoolInitialSize = 10;
	}
}