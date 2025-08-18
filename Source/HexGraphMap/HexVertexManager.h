// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HexCoordinate.h"
#include "HexagonDirection.h"
#include "Vertex.h"
#include "GraphVertex.h"
#include "PlaceHolderVertex.h"
#include "AdjacencyMap.h"
#include "HexVertexManager.generated.h"

class AHexGraph;

/**
 * Manages vertex lifecycle, creation, destruction, and state
 * Handles factory pattern for different vertex types and validation
 */
UCLASS(BlueprintType)
class HEXGRAPHMAP_API UHexVertexManager : public UObject
{
	GENERATED_BODY()

public:
	UHexVertexManager();

	/**
	 * Initialize the vertex manager with required dependencies
	 * @param InOwningHexGraph The HexGraph that owns this manager
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Vertex Manager")
	void Initialize(AHexGraph* InOwningHexGraph);

	/**
	 * Create a vertex at specific coordinate
	 * @param VertexClass Class of vertex to create
	 * @param Coordinate Grid coordinate for the vertex
	 * @param SpawnTransform Transform for spawning
	 * @param bIsTemporary Whether this is a temporary vertex
	 * @return Created vertex or nullptr if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Vertex Manager")
	AVertex* CreateVertex(TSubclassOf<AVertex> VertexClass, const FHexCoordinate& Coordinate, const FTransform& SpawnTransform, bool bIsTemporary = false);

	/**
	 * Create a vertex in a direction from another vertex
	 * @param VertexClass Class of vertex to create
	 * @param RootVertex Reference vertex for positioning
	 * @param Direction Direction from root vertex
	 * @param bIsTemporary Whether this is a temporary vertex
	 * @return Created vertex or nullptr if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Vertex Manager")
	AVertex* CreateVertexInDirection(TSubclassOf<AVertex> VertexClass, AVertex* RootVertex, EHexagonDirection Direction, bool bIsTemporary = false);

	/**
	 * Create a vertex with predefined adjacencies
	 * @param VertexClass Class of vertex to create
	 * @param Coordinate Grid coordinate for the vertex
	 * @param SpawnTransform Transform for spawning
	 * @param Adjacencies Array of adjacent coordinate strings
	 * @param bIsTemporary Whether this is a temporary vertex
	 * @return Created vertex or nullptr if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Vertex Manager")
	AVertex* CreateVertexWithAdjacencies(TSubclassOf<AVertex> VertexClass, const FHexCoordinate& Coordinate, const FTransform& SpawnTransform, const TArray<FString>& Adjacencies, bool bIsTemporary = false);

	/**
	 * Get vertex at specific coordinate
	 * @param Coordinate Grid coordinate to search
	 * @param bIncludeTemporary Whether to include temporary vertices in search
	 * @return Vertex at coordinate or nullptr if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Vertex Manager")
	AVertex* GetVertex(const FHexCoordinate& Coordinate, bool bIncludeTemporary = false) const;

	/**
	 * Remove vertex at specific coordinate
	 * @param Coordinate Grid coordinate of vertex to remove
	 * @param bForceDestroy Whether to destroy even if it should be demoted to placeholder
	 * @return True if vertex was removed/demoted
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Vertex Manager")
	bool RemoveVertex(const FHexCoordinate& Coordinate, bool bForceDestroy = false);

	/**
	 * Promote placeholder vertex to graph vertex
	 * @param PlaceholderVertex Placeholder to promote
	 * @return Promoted graph vertex or nullptr if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Vertex Manager")
	AGraphVertex* PromoteToGraphVertex(APlaceHolderVertex* PlaceholderVertex);

	/**
	 * Demote graph vertex to placeholder vertex
	 * @param GraphVertex Graph vertex to demote
	 * @return Demoted placeholder vertex or nullptr if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Vertex Manager")
	APlaceHolderVertex* DemoteToPlaceholder(AGraphVertex* GraphVertex);

	/**
	 * Get all vertices of a specific type
	 * @param VertexType Type of vertices to retrieve
	 * @param bIncludeTemporary Whether to include temporary vertices
	 * @return Array of vertices matching the type
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Vertex Manager")
	TArray<AVertex*> GetVerticesByType(EVertexType VertexType, bool bIncludeTemporary = false) const;

	/**
	 * Get all vertices within a radius of a coordinate
	 * @param CenterCoordinate Center point for search
	 * @param Radius Search radius in grid units
	 * @param bIncludeTemporary Whether to include temporary vertices
	 * @return Array of vertices within radius
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Vertex Manager")
	TArray<AVertex*> GetVerticesInRadius(const FHexCoordinate& CenterCoordinate, int32 Radius, bool bIncludeTemporary = false) const;

	/**
	 * Validate all vertex references and clean up invalid ones
	 * @return Number of invalid references cleaned up
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Vertex Manager")
	int32 ValidateAndCleanupVertices();

	/**
	 * Clear all temporary vertices
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Vertex Manager")
	void ClearTemporaryVertices();

	/**
	 * Get total vertex count
	 * @param bIncludeTemporary Whether to include temporary vertices in count
	 * @return Total number of vertices
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Vertex Manager")
	int32 GetVertexCount(bool bIncludeTemporary = false) const;

protected:
	/** Reference to the owning HexGraph */
	UPROPERTY()
	TObjectPtr<AHexGraph> OwningHexGraph;

	/** Map of permanent vertices by coordinate */
	UPROPERTY()
	TMap<FString, TObjectPtr<AVertex>> PermanentVertices;

	/** Map of temporary vertices by coordinate */
	UPROPERTY()
	TMap<FString, TObjectPtr<AVertex>> TemporaryVertices;

	/** Adjacency matrices for permanent vertices */
	UPROPERTY()
	TMap<FString, TObjectPtr<UAdjacencyMap>> PermanentAdjacencyMaps;

	/** Adjacency matrices for temporary vertices */
	UPROPERTY()
	TMap<FString, TObjectPtr<UAdjacencyMap>> TemporaryAdjacencyMaps;

	/** Managed adjacency maps for proper cleanup */
	UPROPERTY()
	TArray<TObjectPtr<UAdjacencyMap>> ManagedAdjacencyMaps;

private:
	/**
	 * Initialize a vertex with coordinate and adjacency information
	 * @param Vertex Vertex to initialize
	 * @param Coordinate Grid coordinate
	 * @param bIsTemporary Whether this is a temporary vertex
	 * @param AdjacencyMap Optional pre-created adjacency map
	 */
	void InitializeVertexInternal(AVertex* Vertex, const FHexCoordinate& Coordinate, bool bIsTemporary, UAdjacencyMap* AdjacencyMap = nullptr);

	/**
	 * Create managed adjacency map for proper cleanup
	 * @return New adjacency map or nullptr if failed
	 */
	UAdjacencyMap* CreateManagedAdjacencyMap();

	/**
	 * Get vertex map for coordinate operations
	 * @param bTemporary Whether to use temporary vertex map
	 * @return Reference to appropriate vertex map
	 */
	TMap<FString, TObjectPtr<AVertex>>& GetVertexMap(bool bTemporary);
	const TMap<FString, TObjectPtr<AVertex>>& GetVertexMap(bool bTemporary) const;

	/**
	 * Get adjacency map for coordinate operations
	 * @param bTemporary Whether to use temporary adjacency map
	 * @return Reference to appropriate adjacency map
	 */
	TMap<FString, TObjectPtr<UAdjacencyMap>>& GetAdjacencyMap(bool bTemporary);
	const TMap<FString, TObjectPtr<UAdjacencyMap>>& GetAdjacencyMap(bool bTemporary) const;

	/**
	 * Build spawn parameters for vertex creation
	 * @return Configured spawn parameters
	 */
	FActorSpawnParameters BuildVertexSpawnParams() const;

	/**
	 * Calculate world position for coordinate using current HexGraph settings
	 * @param Coordinate Grid coordinate
	 * @return World position
	 */
	FVector CalculateWorldPosition(const FHexCoordinate& Coordinate) const;
};