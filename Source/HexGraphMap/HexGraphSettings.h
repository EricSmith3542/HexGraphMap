// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/EngineTypes.h"
#include "HexGraphSettings.generated.h"

/**
 * Settings class for HexGraph system configuration
 * Provides centralized configuration management for the hex graph system
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Hex Graph Settings"))
class HEXGRAPHMAP_API UHexGraphSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UHexGraphSettings();

	// Graph Configuration
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Graph", meta = (ClampMin = "0.1", ClampMax = "1000.0"))
	float DefaultVertexSpacing = 100.0f;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Graph", meta = (ClampMin = "1", ClampMax = "100"))
	int32 MaxFillDepth = 5;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Graph", meta = (ClampMin = "1.0", ClampMax = "1000.0"))
	float DefaultMeshLength = 100.0f;

	// Camera Configuration
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "1.0", ClampMax = "100.0"))
	float DefaultPanSpeed = 15.0f;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "10.0", ClampMax = "1000.0"))
	float DefaultZoomPercent = 100.0f;

	// Memory Management Configuration
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Memory", meta = (ClampMin = "100", ClampMax = "100000"))
	int32 ExpectedMaxVertices = 10000;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Memory", meta = (ClampMin = "100", ClampMax = "100000"))
	int32 ExpectedMaxAdjacencyMaps = 10000;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Memory")
	bool bAutoCleanupEnabled = true;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Memory", meta = (ClampMin = "1.0", ClampMax = "60.0"))
	float CleanupIntervalSeconds = 10.0f;

	// Validation Configuration
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Validation", meta = (ClampMin = "-100000", ClampMax = "100000"))
	int32 MaxCoordinateValue = 10000;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Validation", meta = (ClampMin = "-100000", ClampMax = "100000"))
	int32 MinCoordinateValue = -10000;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Validation")
	bool bStrictValidationEnabled = true;

	// Debug Configuration
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bUseMouseFollower = false;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bEnableVerboseLogging = false;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bShowValidationWarnings = true;

	// Performance Configuration
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Performance")
	bool bEnableObjectPooling = false;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (ClampMin = "10", ClampMax = "1000"))
	int32 ObjectPoolInitialSize = 100;

	/**
	 * Get the singleton instance of HexGraphSettings
	 * @return The settings instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Graph|Settings", CallInEditor)
	static const UHexGraphSettings* GetHexGraphSettings();

	/**
	 * Get the settings as a mutable object for runtime changes
	 * @return The mutable settings instance
	 */
	static UHexGraphSettings* GetMutableHexGraphSettings();

	// UDeveloperSettings interface
	virtual FName GetCategoryName() const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	void ValidateSettings();
};