// Copyright Epic Games, Inc. All Rights Reserved.

#include "HexVertexManager.h"
#include "HexGraph.h"
#include "HexCoordinateUtils.h"
#include "HexGraphValidation.h"
#include "HexGraphSettings.h"
#include "HexGraphMap.h"
#include "HexGraphEventManager.h"
#include "Engine/World.h"

UHexVertexManager::UHexVertexManager()
{
	OwningHexGraph = nullptr;
}

void UHexVertexManager::Initialize(AHexGraph* InOwningHexGraph)
{
	OwningHexGraph = InOwningHexGraph;
	
	if (!OwningHexGraph.Get())
	{
		UE_LOG(LogHexGraph, Error, TEXT("HexVertexManager::Initialize: OwningHexGraph is null"));
		return;
	}

	UE_LOG(LogHexGraph, Log, TEXT("HexVertexManager: Initialized with HexGraph '%s'"), *OwningHexGraph->GetName());
}

AVertex* UHexVertexManager::CreateVertex(TSubclassOf<AVertex> VertexClass, const FHexCoordinate& Coordinate, const FTransform& SpawnTransform, bool bIsTemporary)
{
	if (!OwningHexGraph.Get() || !OwningHexGraph->GetWorld())
	{
		UE_LOG(LogHexGraph, Error, TEXT("HexVertexManager::CreateVertex: Invalid HexGraph or World"));
		return nullptr;
	}

	if (!VertexClass)
	{
		UE_LOG(LogHexGraph, Error, TEXT("HexVertexManager::CreateVertex: VertexClass is null"));
		return nullptr;
	}

	if (!Coordinate.IsValid())
	{
		UE_LOG(LogHexGraph, Error, TEXT("HexVertexManager::CreateVertex: Invalid coordinate %s"), *Coordinate.ToString());
		return nullptr;
	}

	// Check if vertex already exists at this coordinate
	// For temporary vertices, only block if another temporary vertex exists
	// For permanent vertices, block if any vertex exists
	bool bShouldBlock = false;
	if (bIsTemporary)
	{
		// For temporary vertices, only check temporary vertices
		AVertex* ExistingTempVertex = GetVertex(Coordinate, false); // Check only temporary
		if (ExistingTempVertex && ExistingTempVertex->type == EVertexType::Preview)
		{
			bShouldBlock = true;
		}
	}
	else
	{
		// For permanent vertices, check all vertices
		if (GetVertex(Coordinate, true))
		{
			bShouldBlock = true;
		}
	}
	
	if (bShouldBlock)
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexVertexManager::CreateVertex: Vertex already exists at %s"), *Coordinate.ToString());
		return nullptr;
	}

	// Create the vertex
	FActorSpawnParameters SpawnParams = BuildVertexSpawnParams();
	AVertex* NewVertex = OwningHexGraph->GetWorld()->SpawnActor<AVertex>(VertexClass, SpawnTransform, SpawnParams);
	
	if (!NewVertex)
	{
		UE_LOG(LogHexGraph, Error, TEXT("HexVertexManager::CreateVertex: Failed to spawn vertex at %s"), *Coordinate.ToString());
		return nullptr;
	}

	// Initialize the vertex
	InitializeVertexInternal(NewVertex, Coordinate, bIsTemporary);

	// Broadcast vertex creation event
	if (OwningHexGraph.Get() && OwningHexGraph->EventManager)
	{
		OwningHexGraph->EventManager->BroadcastEvent(
			EHexGraphEventType::VertexCreated, 
			Coordinate, 
			NewVertex, 
			OwningHexGraph.Get(), 
			bIsTemporary ? TEXT("Temporary vertex created") : TEXT("Permanent vertex created")
		);
	}

	UE_LOG(LogHexGraph, VeryVerbose, TEXT("HexVertexManager::CreateVertex: Created %s vertex at %s"), 
		   bIsTemporary ? TEXT("temporary") : TEXT("permanent"), *Coordinate.ToString());

	return NewVertex;
}

AVertex* UHexVertexManager::CreateVertexInDirection(TSubclassOf<AVertex> VertexClass, AVertex* RootVertex, EHexagonDirection Direction, bool bIsTemporary)
{
	if (!RootVertex)
	{
		UE_LOG(LogHexGraph, Error, TEXT("HexVertexManager::CreateVertexInDirection: RootVertex is null"));
		return nullptr;
	}

	// Get the coordinate in the specified direction
	FHexCoordinate RootCoord = FHexCoordinate::FromString(RootVertex->Coord());
	FHexCoordinate TargetCoord = FHexCoordinateUtils::GetCoordinateInDirection(RootCoord, Direction);

	// Calculate transform based on direction and HexGraph settings
	FTransform NewTransform = RootVertex->GetActorTransform();
	FVector DirectionOffset = CalculateWorldPosition(TargetCoord) - CalculateWorldPosition(RootCoord);
	NewTransform.AddToTranslation(DirectionOffset);

	return CreateVertex(VertexClass, TargetCoord, NewTransform, bIsTemporary);
}

AVertex* UHexVertexManager::CreateVertexWithAdjacencies(TSubclassOf<AVertex> VertexClass, const FHexCoordinate& Coordinate, const FTransform& SpawnTransform, const TArray<FString>& Adjacencies, bool bIsTemporary)
{
	AVertex* NewVertex = CreateVertex(VertexClass, Coordinate, SpawnTransform, bIsTemporary);
	
	if (!NewVertex)
	{
		return nullptr;
	}

	// Set up adjacencies
	FString CoordString = Coordinate.ToString();
	TMap<FString, TObjectPtr<UAdjacencyMap>>& AdjMap = GetAdjacencyMap(bIsTemporary);
	
	if (TObjectPtr<UAdjacencyMap>* AdjacencyMapPtr = AdjMap.Find(CoordString))
	{
		if (AdjacencyMapPtr->Get())
		{
			AdjacencyMapPtr->Get()->setAllAdjacencies(Adjacencies);
		}
	}

	return NewVertex;
}

AVertex* UHexVertexManager::GetVertex(const FHexCoordinate& Coordinate, bool bIncludeTemporary) const
{
	FString CoordString = Coordinate.ToString();

	// Check permanent vertices first
	if (const TObjectPtr<AVertex>* VertexPtr = PermanentVertices.Find(CoordString))
	{
		if (IsValid(VertexPtr->Get()))
		{
			return VertexPtr->Get();
		}
	}

	// Check temporary vertices if requested
	if (bIncludeTemporary)
	{
		if (const TObjectPtr<AVertex>* VertexPtr = TemporaryVertices.Find(CoordString))
		{
			if (IsValid(VertexPtr->Get()))
			{
				return VertexPtr->Get();
			}
		}
	}

	return nullptr;
}

bool UHexVertexManager::RemoveVertex(const FHexCoordinate& Coordinate, bool bForceDestroy)
{
	FString CoordString = Coordinate.ToString();
	AVertex* VertexToRemove = GetVertex(Coordinate, true);
	
	if (!VertexToRemove)
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexVertexManager::RemoveVertex: No vertex found at %s"), *CoordString);
		return false;
	}

	bool bIsTemporary = TemporaryVertices.Contains(CoordString);
	
	// Handle graph vertex demotion logic (unless forced)
	if (!bForceDestroy && VertexToRemove->type == EVertexType::Graph)
	{
		// Check if vertex should be demoted instead of destroyed
		TArray<FHexCoordinate> AdjacentCoords = FHexCoordinateUtils::GetAdjacentCoordinates(Coordinate);
		bool bHasGraphNeighbors = false;
		
		for (const FHexCoordinate& AdjCoord : AdjacentCoords)
		{
			if (AVertex* Neighbor = GetVertex(AdjCoord))
			{
				if (Neighbor->type == EVertexType::Graph)
				{
					bHasGraphNeighbors = true;
					break;
				}
			}
		}

		if (bHasGraphNeighbors)
		{
			// Demote to placeholder instead of destroying
			AGraphVertex* GraphVertex = Cast<AGraphVertex>(VertexToRemove);
			if (GraphVertex)
			{
				DemoteToPlaceholder(GraphVertex);
				return true;
			}
		}
	}

	// Remove from appropriate maps
	TMap<FString, TObjectPtr<AVertex>>& VertexMap = GetVertexMap(bIsTemporary);
	TMap<FString, TObjectPtr<UAdjacencyMap>>& AdjMap = GetAdjacencyMap(bIsTemporary);
	
	VertexMap.Remove(CoordString);
	AdjMap.Remove(CoordString);

	// Clean up adjacency references in neighbors
	TArray<FHexCoordinate> AdjacentCoords = FHexCoordinateUtils::GetAdjacentCoordinates(Coordinate);
	for (const FHexCoordinate& AdjCoord : AdjacentCoords)
	{
		// Remove references to this vertex from neighbors
		// This would need to be implemented based on the adjacency system
	}

	// Broadcast vertex destruction event before destroying
	if (OwningHexGraph.Get() && OwningHexGraph->EventManager)
	{
		OwningHexGraph->EventManager->BroadcastEvent(
			EHexGraphEventType::VertexDestroyed, 
			Coordinate, 
			VertexToRemove, 
			OwningHexGraph.Get(), 
			FString::Printf(TEXT("Vertex removed at %s"), *CoordString)
		);
	}

	// Destroy the vertex
	VertexToRemove->Destroy();

	UE_LOG(LogHexGraph, Log, TEXT("HexVertexManager::RemoveVertex: Removed vertex at %s"), *CoordString);
	return true;
}

AGraphVertex* UHexVertexManager::PromoteToGraphVertex(APlaceHolderVertex* PlaceholderVertex)
{
	if (!PlaceholderVertex || !OwningHexGraph.Get())
	{
		return nullptr;
	}

	FHexCoordinate Coordinate = FHexCoordinate::FromString(PlaceholderVertex->Coord());
	FTransform VertexTransform = PlaceholderVertex->GetActorTransform();
	
	// Store the adjacency map before destroying placeholder
	FString CoordString = Coordinate.ToString();
	UAdjacencyMap* ExistingAdjacencyMap = nullptr;
	if (TObjectPtr<UAdjacencyMap>* AdjMapPtr = PermanentAdjacencyMaps.Find(CoordString))
	{
		ExistingAdjacencyMap = AdjMapPtr->Get();
	}

	// Remove placeholder from maps (but don't destroy adjacency map)
	PermanentVertices.Remove(CoordString);
	
	// Create new graph vertex
	AGraphVertex* NewGraphVertex = OwningHexGraph->GetWorld()->SpawnActor<AGraphVertex>(
		OwningHexGraph->defaultVertexClass, VertexTransform, BuildVertexSpawnParams());
	
	if (!NewGraphVertex)
	{
		UE_LOG(LogHexGraph, Error, TEXT("HexVertexManager::PromoteToGraphVertex: Failed to create graph vertex"));
		return nullptr;
	}

	// Initialize and add to maps
	InitializeVertexInternal(NewGraphVertex, Coordinate, false, ExistingAdjacencyMap);
	
	// Store adjacency information before destroying placeholder
	TArray<FString> neighbors;
	if (ExistingAdjacencyMap)
	{
		neighbors = ExistingAdjacencyMap->adjacentVertexCoords;
	}
	else
	{
		neighbors = { "", "", "", "", "", "" }; // Empty adjacencies
	}
	
	// Destroy the placeholder
	PlaceholderVertex->Destroy();

	// Build new placeholders in all empty neighbor coordinates (original behavior)
	for (int i = 0; i < neighbors.Num(); i++)
	{
		if (neighbors[i] == "") 
		{
			EHexagonDirection Direction = static_cast<EHexagonDirection>(i);
			AVertex* newPlaceHolder = CreateVertexInDirection(
				OwningHexGraph->placeHolderVertexClass, 
				NewGraphVertex, 
				Direction, 
				false
			);

			if (newPlaceHolder)
			{
				//Check all directions for adjacencies
				TArray<FString> placeHolderAdjacencies = { "", "", "", "", "", "" };
				for (int j = 0; j < 6; j++)
				{
					FHexCoordinate NeighborCoord = FHexCoordinateUtils::GetCoordinateInDirection(
						FHexCoordinate::FromString(newPlaceHolder->Coord()), 
						static_cast<EHexagonDirection>(j)
					);
					FString coord = NeighborCoord.ToString();

					//Ensure bidirectional adjacency with all other vertices
					if (AVertex* ExistingVertex = GetVertex(NeighborCoord, false))
					{
						placeHolderAdjacencies[j] = coord;
						
						// Set bidirectional adjacency
						TMap<FString, TObjectPtr<UAdjacencyMap>>& PermanentAdjMap = GetAdjacencyMap(false);
						if (TObjectPtr<UAdjacencyMap>* ExistingAdjMapPtr = PermanentAdjMap.Find(coord))
						{
							if (ExistingAdjMapPtr->Get())
							{
								ExistingAdjMapPtr->Get()->setDirectionsAdjacency((j + 3) % 6, newPlaceHolder->Coord());
							}
						}
					}
				}
				
				// Set adjacencies for the new placeholder
				TMap<FString, TObjectPtr<UAdjacencyMap>>& PlaceholderAdjMap = GetAdjacencyMap(false);
				if (TObjectPtr<UAdjacencyMap>* NewPlaceHolderAdjMapPtr = PlaceholderAdjMap.Find(newPlaceHolder->Coord()))
				{
					if (NewPlaceHolderAdjMapPtr->Get())
					{
						NewPlaceHolderAdjMapPtr->Get()->setAllAdjacencies(placeHolderAdjacencies);
					}
				}
				
				// Set the promoted vertex's adjacency to the new placeholder
				TMap<FString, TObjectPtr<UAdjacencyMap>>& PromotedAdjMap = GetAdjacencyMap(false);
				if (TObjectPtr<UAdjacencyMap>* PromotedVertexAdjMapPtr = PromotedAdjMap.Find(CoordString))
				{
					if (PromotedVertexAdjMapPtr->Get())
					{
						PromotedVertexAdjMapPtr->Get()->setDirectionsAdjacency(i, newPlaceHolder->Coord());
					}
				}
			}
		}
	}

	UE_LOG(LogHexGraph, Log, TEXT("HexVertexManager::PromoteToGraphVertex: Promoted vertex at %s"), *CoordString);
	return NewGraphVertex;
}

APlaceHolderVertex* UHexVertexManager::DemoteToPlaceholder(AGraphVertex* GraphVertex)
{
	if (!GraphVertex || !OwningHexGraph.Get())
	{
		return nullptr;
	}

	FHexCoordinate Coordinate = FHexCoordinate::FromString(GraphVertex->Coord());
	FTransform VertexTransform = GraphVertex->GetActorTransform();
	
	// Store the adjacency map before destroying graph vertex
	FString CoordString = Coordinate.ToString();
	UAdjacencyMap* ExistingAdjacencyMap = nullptr;
	if (TObjectPtr<UAdjacencyMap>* AdjMapPtr = PermanentAdjacencyMaps.Find(CoordString))
	{
		ExistingAdjacencyMap = AdjMapPtr->Get();
	}

	// Remove graph vertex from maps
	PermanentVertices.Remove(CoordString);
	
	// Create new placeholder vertex
	APlaceHolderVertex* NewPlaceholder = OwningHexGraph->GetWorld()->SpawnActor<APlaceHolderVertex>(
		OwningHexGraph->placeHolderVertexClass, VertexTransform, BuildVertexSpawnParams());
	
	if (!NewPlaceholder)
	{
		UE_LOG(LogHexGraph, Error, TEXT("HexVertexManager::DemoteToPlaceholder: Failed to create placeholder vertex"));
		return nullptr;
	}

	// Initialize and add to maps
	InitializeVertexInternal(NewPlaceholder, Coordinate, false, ExistingAdjacencyMap);
	
	// Destroy the graph vertex
	GraphVertex->Destroy();

	UE_LOG(LogHexGraph, Log, TEXT("HexVertexManager::DemoteToPlaceholder: Demoted vertex at %s"), *CoordString);
	return NewPlaceholder;
}

TArray<AVertex*> UHexVertexManager::GetVerticesByType(EVertexType VertexType, bool bIncludeTemporary) const
{
	TArray<AVertex*> MatchingVertices;

	// Check permanent vertices
	for (const auto& VertexPair : PermanentVertices)
	{
		if (IsValid(VertexPair.Value.Get()) && VertexPair.Value->type == VertexType)
		{
			MatchingVertices.Add(VertexPair.Value.Get());
		}
	}

	// Check temporary vertices if requested
	if (bIncludeTemporary)
	{
		for (const auto& VertexPair : TemporaryVertices)
		{
			if (IsValid(VertexPair.Value.Get()) && VertexPair.Value->type == VertexType)
			{
				MatchingVertices.Add(VertexPair.Value.Get());
			}
		}
	}

	return MatchingVertices;
}

TArray<AVertex*> UHexVertexManager::GetVerticesInRadius(const FHexCoordinate& CenterCoordinate, int32 Radius, bool bIncludeTemporary) const
{
	TArray<AVertex*> VerticesInRadius;
	TArray<FHexCoordinate> CoordinatesInRadius = FHexCoordinateUtils::GetCoordinatesInRadius(CenterCoordinate, Radius);

	for (const FHexCoordinate& Coordinate : CoordinatesInRadius)
	{
		if (AVertex* Vertex = GetVertex(Coordinate, bIncludeTemporary))
		{
			VerticesInRadius.Add(Vertex);
		}
	}

	return VerticesInRadius;
}

int32 UHexVertexManager::ValidateAndCleanupVertices()
{
	int32 CleanedCount = 0;

	// Clean permanent vertices
	TArray<FString> InvalidKeys;
	for (const auto& VertexPair : PermanentVertices)
	{
		if (!IsValid(VertexPair.Value.Get()))
		{
			InvalidKeys.Add(VertexPair.Key);
		}
	}

	for (const FString& Key : InvalidKeys)
	{
		PermanentVertices.Remove(Key);
		PermanentAdjacencyMaps.Remove(Key);
		CleanedCount++;
	}

	// Clean temporary vertices
	InvalidKeys.Empty();
	for (const auto& VertexPair : TemporaryVertices)
	{
		if (!IsValid(VertexPair.Value.Get()))
		{
			InvalidKeys.Add(VertexPair.Key);
		}
	}

	for (const FString& Key : InvalidKeys)
	{
		TemporaryVertices.Remove(Key);
		TemporaryAdjacencyMaps.Remove(Key);
		CleanedCount++;
	}

	UE_LOG(LogHexGraph, Log, TEXT("HexVertexManager::ValidateAndCleanupVertices: Cleaned %d invalid references"), CleanedCount);
	return CleanedCount;
}

void UHexVertexManager::ClearTemporaryVertices()
{
	for (const auto& VertexPair : TemporaryVertices)
	{
		if (IsValid(VertexPair.Value.Get()))
		{
			VertexPair.Value->Destroy();
		}
	}

	TemporaryVertices.Empty();
	TemporaryAdjacencyMaps.Empty();

	UE_LOG(LogHexGraph, Log, TEXT("HexVertexManager::ClearTemporaryVertices: Cleared all temporary vertices"));
}

int32 UHexVertexManager::GetVertexCount(bool bIncludeTemporary) const
{
	int32 Count = PermanentVertices.Num();
	if (bIncludeTemporary)
	{
		Count += TemporaryVertices.Num();
	}
	return Count;
}

void UHexVertexManager::InitializeVertexInternal(AVertex* Vertex, const FHexCoordinate& Coordinate, bool bIsTemporary, UAdjacencyMap* AdjacencyMap)
{
	if (!Vertex)
	{
		UE_LOG(LogHexGraph, Error, TEXT("HexVertexManager::InitializeVertexInternal: Vertex is null"));
		return;
	}

	// Set vertex coordinates
	Vertex->row = Coordinate.Row;
	Vertex->col = Coordinate.Col;

	FString CoordString = Coordinate.ToString();
	
	// Create or use provided adjacency map
	UAdjacencyMap* AdjMap = AdjacencyMap;
	if (!AdjMap)
	{
		AdjMap = CreateManagedAdjacencyMap();
	}

	// Add to appropriate maps
	TMap<FString, TObjectPtr<AVertex>>& VertexMap = GetVertexMap(bIsTemporary);
	TMap<FString, TObjectPtr<UAdjacencyMap>>& AdjMapContainer = GetAdjacencyMap(bIsTemporary);
	
	VertexMap.Add(CoordString, Vertex);
	AdjMapContainer.Add(CoordString, AdjMap);

	// Set up event handlers if OwningHexGraph is available
	if (OwningHexGraph.Get())
	{
		Vertex->OnBeginCursorOver.AddDynamic(OwningHexGraph.Get(), &AHexGraph::OnVertexHoverBegin);
		Vertex->OnEndCursorOver.AddDynamic(OwningHexGraph.Get(), &AHexGraph::OnVertexHoverEnd);
	}

	UE_LOG(LogHexGraph, VeryVerbose, TEXT("HexVertexManager::InitializeVertexInternal: Initialized vertex at %s"), *CoordString);
}

UAdjacencyMap* UHexVertexManager::CreateManagedAdjacencyMap()
{
	if (!OwningHexGraph.Get())
	{
		UE_LOG(LogHexGraph, Error, TEXT("HexVertexManager::CreateManagedAdjacencyMap: OwningHexGraph is null"));
		return nullptr;
	}

	UAdjacencyMap* NewAdjMap = NewObject<UAdjacencyMap>(OwningHexGraph.Get());
	if (NewAdjMap)
	{
		ManagedAdjacencyMaps.Add(NewAdjMap);
	}

	return NewAdjMap;
}

TMap<FString, TObjectPtr<AVertex>>& UHexVertexManager::GetVertexMap(bool bTemporary)
{
	return bTemporary ? TemporaryVertices : PermanentVertices;
}

const TMap<FString, TObjectPtr<AVertex>>& UHexVertexManager::GetVertexMap(bool bTemporary) const
{
	return bTemporary ? TemporaryVertices : PermanentVertices;
}

TMap<FString, TObjectPtr<UAdjacencyMap>>& UHexVertexManager::GetAdjacencyMap(bool bTemporary)
{
	return bTemporary ? TemporaryAdjacencyMaps : PermanentAdjacencyMaps;
}

const TMap<FString, TObjectPtr<UAdjacencyMap>>& UHexVertexManager::GetAdjacencyMap(bool bTemporary) const
{
	return bTemporary ? TemporaryAdjacencyMaps : PermanentAdjacencyMaps;
}

FActorSpawnParameters UHexVertexManager::BuildVertexSpawnParams() const
{
	static int32 VertexCounter = 0;
	
	FActorSpawnParameters Params;
	Params.Name = FName(FString::Printf(TEXT("Vertex-%d"), ++VertexCounter));
	Params.Owner = OwningHexGraph.Get();
	
	return Params;
}

FVector UHexVertexManager::CalculateWorldPosition(const FHexCoordinate& Coordinate) const
{
	if (!OwningHexGraph.Get())
	{
		return FVector::ZeroVector;
	}

	return FHexCoordinateUtils::CoordinateToWorldPosition(Coordinate, OwningHexGraph->MeshLength, OwningHexGraph->VertexSpacing);
}