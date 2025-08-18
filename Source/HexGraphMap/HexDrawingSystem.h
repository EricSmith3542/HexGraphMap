// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HexCoordinate.h"
#include "HexagonDirection.h"
#include "Vertex.h"
#include "PlaceHolderVertex.h"
#include "HexDrawingSystem.generated.h"

class AHexGraph;
class UHexVertexManager;

/**
 * Drawing modes for different interaction types
 */
UENUM(BlueprintType)
enum class EHexDrawingMode : uint8
{
	None		UMETA(DisplayName = "None"),
	LineDraw	UMETA(DisplayName = "Line Drawing"),
	FillArea	UMETA(DisplayName = "Fill Area"),
	PieceDraw	UMETA(DisplayName = "Piece Drawing")
};

/**
 * Drawing state for tracking current operation
 */
UENUM(BlueprintType)
enum class EHexDrawingState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Active		UMETA(DisplayName = "Active"),
	Preview		UMETA(DisplayName = "Preview"),
	Committing	UMETA(DisplayName = "Committing")
};

/**
 * Delegate signatures for drawing events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDrawingStartEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDrawingStopEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDrawingPreviewEvent, const TArray<FHexCoordinate>&, PreviewCoordinates);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDrawingCommitEvent, const TArray<FHexCoordinate>&, CommittedCoordinates);

/**
 * Manages line drawing, preview generation, and temporary vertex operations
 * Handles different drawing modes and state management
 */
UCLASS(BlueprintType)
class HEXGRAPHMAP_API UHexDrawingSystem : public UObject
{
	GENERATED_BODY()

public:
	UHexDrawingSystem();

	/**
	 * Initialize the drawing system with required dependencies
	 * @param InOwningHexGraph The HexGraph that owns this system
	 * @param InVertexManager The vertex manager to use for operations
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Drawing System")
	void Initialize(AHexGraph* InOwningHexGraph, UHexVertexManager* InVertexManager);

	/**
	 * Start drawing operation
	 * @param Mode Drawing mode to start
	 * @param StartCoordinate Starting coordinate for the drawing
	 * @return True if drawing started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Drawing System")
	bool StartDrawing(EHexDrawingMode Mode, const FHexCoordinate& StartCoordinate);

	/**
	 * Stop current drawing operation
	 * @param bCommitChanges Whether to commit the drawing or cancel it
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Drawing System")
	void StopDrawing(bool bCommitChanges = true);

	/**
	 * Update drawing preview based on current mouse/target position
	 * @param TargetCoordinate Current target coordinate
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Drawing System")
	void UpdatePreview(const FHexCoordinate& TargetCoordinate);

	/**
	 * Clear all temporary/preview elements
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Drawing System")
	void ClearPreview();

	/**
	 * Commit current drawing to permanent vertices
	 * @return Array of coordinates that were committed
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Drawing System")
	TArray<FHexCoordinate> CommitDrawing();

	/**
	 * Cancel current drawing operation
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Drawing System")
	void CancelDrawing();

	/**
	 * Get current drawing mode
	 * @return Current drawing mode
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Drawing System")
	EHexDrawingMode GetDrawingMode() const { return CurrentDrawingMode; }

	/**
	 * Get current drawing state
	 * @return Current drawing state
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Drawing System")
	EHexDrawingState GetDrawingState() const { return CurrentDrawingState; }

	/**
	 * Check if drawing is currently active
	 * @return True if drawing is active
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Drawing System")
	bool IsDrawingActive() const { return CurrentDrawingState != EHexDrawingState::Idle; }

	/**
	 * Get current preview coordinates
	 * @return Array of coordinates being previewed
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Drawing System")
	TArray<FHexCoordinate> GetPreviewCoordinates() const { return PreviewCoordinates; }

	/**
	 * Set maximum drawing distance for line drawing
	 * @param MaxDistance Maximum distance in grid units
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Drawing System")
	void SetMaxDrawingDistance(int32 MaxDistance) { MaxLineDrawDistance = MaxDistance; }

	/**
	 * Process fill operation at specified coordinate
	 * @param CenterCoordinate Center coordinate for fill
	 * @param MaxDepth Maximum fill depth
	 * @return Array of coordinates that were filled
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Drawing System")
	TArray<FHexCoordinate> ProcessFill(const FHexCoordinate& CenterCoordinate, int32 MaxDepth);

	// Drawing Events
	UPROPERTY(BlueprintAssignable, Category = "Drawing Events")
	FOnDrawingStartEvent OnDrawingStartEvent;

	UPROPERTY(BlueprintAssignable, Category = "Drawing Events")
	FOnDrawingStopEvent OnDrawingStopEvent;

	UPROPERTY(BlueprintAssignable, Category = "Drawing Events")
	FOnDrawingPreviewEvent OnDrawingPreviewEvent;

	UPROPERTY(BlueprintAssignable, Category = "Drawing Events")
	FOnDrawingCommitEvent OnDrawingCommitEvent;

protected:
	/** Reference to the owning HexGraph */
	UPROPERTY()
	TObjectPtr<AHexGraph> OwningHexGraph;

	/** Reference to the vertex manager */
	UPROPERTY()
	TObjectPtr<UHexVertexManager> VertexManager;

	/** Current drawing mode */
	UPROPERTY(BlueprintReadOnly, Category = "Drawing State")
	EHexDrawingMode CurrentDrawingMode;

	/** Current drawing state */
	UPROPERTY(BlueprintReadOnly, Category = "Drawing State")
	EHexDrawingState CurrentDrawingState;

	/** Starting coordinate for current drawing operation */
	UPROPERTY(BlueprintReadOnly, Category = "Drawing State")
	FHexCoordinate StartCoordinate;

	/** Current target coordinate */
	UPROPERTY(BlueprintReadOnly, Category = "Drawing State")
	FHexCoordinate CurrentTargetCoordinate;

	/** Array of coordinates being previewed */
	UPROPERTY(BlueprintReadOnly, Category = "Drawing State")
	TArray<FHexCoordinate> PreviewCoordinates;

	/** Array of preview vertices created */
	UPROPERTY()
	TArray<TObjectPtr<AVertex>> PreviewVertices;

	/** Last preview vertex for line drawing continuation */
	UPROPERTY()
	TObjectPtr<AVertex> LastPreviewVertex;

	/** Maximum distance for line drawing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Settings")
	int32 MaxLineDrawDistance;

	/** Whether to show preview vertices */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Settings")
	bool bShowPreviewVertices;

	/** Preview vertex class to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Settings")
	TSubclassOf<AVertex> PreviewVertexClass;

private:
	/**
	 * Generate line coordinates between start and target
	 * @param Start Starting coordinate
	 * @param Target Target coordinate
	 * @return Array of coordinates forming the line
	 */
	TArray<FHexCoordinate> GenerateLineCoordinates(const FHexCoordinate& Start, const FHexCoordinate& Target) const;

	/**
	 * Generate fill coordinates around a center point
	 * @param Center Center coordinate
	 * @param MaxDepth Maximum depth to fill
	 * @return Array of coordinates to fill
	 */
	TArray<FHexCoordinate> GenerateFillCoordinates(const FHexCoordinate& Center, int32 MaxDepth) const;

	/**
	 * Create preview vertices for given coordinates
	 * @param Coordinates Coordinates to create previews for
	 */
	void CreatePreviewVertices(const TArray<FHexCoordinate>& Coordinates);

	/**
	 * Destroy all current preview vertices
	 */
	void DestroyPreviewVertices();

	/**
	 * Update drawing state
	 * @param NewState New state to set
	 */
	void SetDrawingState(EHexDrawingState NewState);

	/**
	 * Validate that coordinates are suitable for drawing
	 * @param Coordinates Coordinates to validate
	 * @return Array of valid coordinates
	 */
	TArray<FHexCoordinate> ValidateDrawingCoordinates(const TArray<FHexCoordinate>& Coordinates) const;

	/**
	 * Get world position for coordinate using current settings
	 * @param Coordinate Grid coordinate
	 * @return World position
	 */
	FVector GetWorldPositionForCoordinate(const FHexCoordinate& Coordinate) const;

	/**
	 * Load drawing settings from HexGraph settings
	 */
	void LoadDrawingSettings();
};