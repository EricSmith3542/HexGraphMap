// Copyright Epic Games, Inc. All Rights Reserved.

#include "HexDrawingSystem.h"
#include "HexGraph.h"
#include "HexVertexManager.h"
#include "HexCoordinateUtils.h"
#include "HexGraphSettings.h"
#include "HexGraphMap.h"
#include "Engine/World.h"

UHexDrawingSystem::UHexDrawingSystem()
{
	OwningHexGraph = nullptr;
	VertexManager = nullptr;
	CurrentDrawingMode = EHexDrawingMode::None;
	CurrentDrawingState = EHexDrawingState::Idle;
	StartCoordinate = FHexCoordinate(0, 0);
	CurrentTargetCoordinate = FHexCoordinate(0, 0);
	LastPreviewVertex = nullptr;
	MaxLineDrawDistance = 20;
	bShowPreviewVertices = true;
	PreviewVertexClass = nullptr;
}

void UHexDrawingSystem::Initialize(AHexGraph* InOwningHexGraph, UHexVertexManager* InVertexManager)
{
	OwningHexGraph = InOwningHexGraph;
	VertexManager = InVertexManager;

	if (!OwningHexGraph.Get())
	{
		UE_LOG(LogHexGraph, Error, TEXT("HexDrawingSystem::Initialize: OwningHexGraph is null"));
		return;
	}

	if (!VertexManager.Get())
	{
		UE_LOG(LogHexGraph, Error, TEXT("HexDrawingSystem::Initialize: VertexManager is null"));
		return;
	}

	// Load settings and configuration
	LoadDrawingSettings();

	UE_LOG(LogHexGraph, Log, TEXT("HexDrawingSystem: Initialized with HexGraph '%s'"), *OwningHexGraph->GetName());
}

bool UHexDrawingSystem::StartDrawing(EHexDrawingMode Mode, const FHexCoordinate& StartCoord)
{
	if (CurrentDrawingState != EHexDrawingState::Idle)
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexDrawingSystem::StartDrawing: Already drawing"));
		return false;
	}

	if (!StartCoord.IsValid())
	{
		UE_LOG(LogHexGraph, Error, TEXT("HexDrawingSystem::StartDrawing: Invalid start coordinate"));
		return false;
	}

	CurrentDrawingMode = Mode;
	StartCoordinate = StartCoord;
	CurrentTargetCoordinate = StartCoord;
	SetDrawingState(EHexDrawingState::Active);

	// Clear any existing preview
	ClearPreview();

	OnDrawingStartEvent.Broadcast();

	UE_LOG(LogHexGraph, Log, TEXT("HexDrawingSystem: Started drawing mode %d at %s"), 
		   static_cast<int32>(Mode), *StartCoord.ToString());

	return true;
}

void UHexDrawingSystem::StopDrawing(bool bCommitChanges)
{
	if (CurrentDrawingState == EHexDrawingState::Idle)
	{
		return;
	}

	if (bCommitChanges && CurrentDrawingState == EHexDrawingState::Active)
	{
		CommitDrawing();
	}
	else
	{
		CancelDrawing();
	}

	OnDrawingStopEvent.Broadcast();

	UE_LOG(LogHexGraph, Log, TEXT("HexDrawingSystem: Stopped drawing - Committed: %s"), 
		   bCommitChanges ? TEXT("true") : TEXT("false"));
}

void UHexDrawingSystem::UpdatePreview(const FHexCoordinate& TargetCoordinate)
{
	if (CurrentDrawingState != EHexDrawingState::Active || !TargetCoordinate.IsValid())
	{
		return;
	}

	// Don't update if target hasn't changed
	if (CurrentTargetCoordinate == TargetCoordinate)
	{
		return;
	}

	CurrentTargetCoordinate = TargetCoordinate;
	SetDrawingState(EHexDrawingState::Preview);

	// Clear existing preview
	DestroyPreviewVertices();
	PreviewCoordinates.Empty();

	// Generate new preview based on drawing mode
	switch (CurrentDrawingMode)
	{
	case EHexDrawingMode::LineDraw:
		PreviewCoordinates = GenerateLineCoordinates(StartCoordinate, TargetCoordinate);
		break;
	case EHexDrawingMode::FillArea:
		PreviewCoordinates = GenerateFillCoordinates(TargetCoordinate, MaxLineDrawDistance);
		break;
	case EHexDrawingMode::PieceDraw:
		// Piece drawing preview would be implemented here
		PreviewCoordinates.Add(TargetCoordinate);
		break;
	default:
		PreviewCoordinates.Add(TargetCoordinate);
		break;
	}

	// Validate coordinates
	PreviewCoordinates = ValidateDrawingCoordinates(PreviewCoordinates);

	// Create preview vertices if enabled
	if (bShowPreviewVertices)
	{
		CreatePreviewVertices(PreviewCoordinates);
	}

	OnDrawingPreviewEvent.Broadcast(PreviewCoordinates);

	UE_LOG(LogHexGraph, VeryVerbose, TEXT("HexDrawingSystem: Updated preview with %d coordinates"), 
		   PreviewCoordinates.Num());
}

void UHexDrawingSystem::ClearPreview()
{
	DestroyPreviewVertices();
	PreviewCoordinates.Empty();
	LastPreviewVertex = nullptr;

	if (CurrentDrawingState == EHexDrawingState::Preview)
	{
		SetDrawingState(EHexDrawingState::Active);
	}

	UE_LOG(LogHexGraph, VeryVerbose, TEXT("HexDrawingSystem: Preview cleared"));
}

TArray<FHexCoordinate> UHexDrawingSystem::CommitDrawing()
{
	if (CurrentDrawingState == EHexDrawingState::Idle || !VertexManager.Get())
	{
		return TArray<FHexCoordinate>();
	}

	SetDrawingState(EHexDrawingState::Committing);

	TArray<FHexCoordinate> CommittedCoordinates;

	// Get coordinates to commit (use preview if available, otherwise just the target)
	TArray<FHexCoordinate> CoordinatesToCommit = PreviewCoordinates.Num() > 0 ? 
		PreviewCoordinates : TArray<FHexCoordinate>({CurrentTargetCoordinate});

	// Create permanent vertices for each coordinate
	for (const FHexCoordinate& Coordinate : CoordinatesToCommit)
	{
		// Check if there's already a vertex at this location
		AVertex* ExistingVertex = VertexManager->GetVertex(Coordinate, false);
		
		if (!ExistingVertex)
		{
			// Create new vertex
			FVector WorldPosition = GetWorldPositionForCoordinate(Coordinate);
			FTransform SpawnTransform(FRotator::ZeroRotator, WorldPosition, FVector::OneVector);
			
			if (OwningHexGraph.Get() && OwningHexGraph->defaultVertexClass)
			{
				AVertex* NewVertex = VertexManager->CreateVertex(
					OwningHexGraph->defaultVertexClass, 
					Coordinate, 
					SpawnTransform, 
					false
				);

				if (NewVertex)
				{
					CommittedCoordinates.Add(Coordinate);
				}
			}
		}
		else if (ExistingVertex->type == EVertexType::PlaceHolder)
		{
			// Promote placeholder to graph vertex
			APlaceHolderVertex* Placeholder = Cast<APlaceHolderVertex>(ExistingVertex);
			if (Placeholder)
			{
				VertexManager->PromoteToGraphVertex(Placeholder);
				CommittedCoordinates.Add(Coordinate);
			}
		}
	}

	// Clean up and reset state
	ClearPreview();
	SetDrawingState(EHexDrawingState::Idle);
	CurrentDrawingMode = EHexDrawingMode::None;

	OnDrawingCommitEvent.Broadcast(CommittedCoordinates);

	UE_LOG(LogHexGraph, Log, TEXT("HexDrawingSystem: Committed %d coordinates"), CommittedCoordinates.Num());

	return CommittedCoordinates;
}

void UHexDrawingSystem::CancelDrawing()
{
	ClearPreview();
	SetDrawingState(EHexDrawingState::Idle);
	CurrentDrawingMode = EHexDrawingMode::None;

	UE_LOG(LogHexGraph, Log, TEXT("HexDrawingSystem: Drawing cancelled"));
}

TArray<FHexCoordinate> UHexDrawingSystem::ProcessFill(const FHexCoordinate& CenterCoordinate, int32 MaxDepth)
{
	if (!VertexManager.Get())
	{
		return TArray<FHexCoordinate>();
	}

	TArray<FHexCoordinate> FilledCoordinates;
	TArray<FHexCoordinate> CoordinatesToProcess;
	TSet<FString> ProcessedCoordinates;

	// Start with center coordinate if it's a placeholder
	AVertex* CenterVertex = VertexManager->GetVertex(CenterCoordinate, false);
	if (CenterVertex && CenterVertex->type == EVertexType::PlaceHolder)
	{
		CoordinatesToProcess.Add(CenterCoordinate);
	}

	// Process fill using breadth-first approach
	for (int32 Depth = 0; Depth < MaxDepth && CoordinatesToProcess.Num() > 0; ++Depth)
	{
		TArray<FHexCoordinate> NextDepthCoordinates;

		for (const FHexCoordinate& Coordinate : CoordinatesToProcess)
		{
			FString CoordString = Coordinate.ToString();
			if (ProcessedCoordinates.Contains(CoordString))
			{
				continue;
			}

			ProcessedCoordinates.Add(CoordString);

			// Check if this coordinate has a placeholder vertex
			AVertex* Vertex = VertexManager->GetVertex(Coordinate, false);
			if (Vertex && Vertex->type == EVertexType::PlaceHolder)
			{
				// Promote to graph vertex
				APlaceHolderVertex* Placeholder = Cast<APlaceHolderVertex>(Vertex);
				if (Placeholder)
				{
					VertexManager->PromoteToGraphVertex(Placeholder);
					FilledCoordinates.Add(Coordinate);

					// Add adjacent placeholders for next depth
					TArray<FHexCoordinate> AdjacentCoords = FHexCoordinateUtils::GetAdjacentCoordinates(Coordinate);
					for (const FHexCoordinate& AdjCoord : AdjacentCoords)
					{
						AVertex* AdjVertex = VertexManager->GetVertex(AdjCoord, false);
						if (AdjVertex && AdjVertex->type == EVertexType::PlaceHolder)
						{
							NextDepthCoordinates.AddUnique(AdjCoord);
						}
					}
				}
			}
		}

		CoordinatesToProcess = NextDepthCoordinates;
	}

	UE_LOG(LogHexGraph, Log, TEXT("HexDrawingSystem: Fill operation completed - Filled %d coordinates"), 
		   FilledCoordinates.Num());

	return FilledCoordinates;
}

TArray<FHexCoordinate> UHexDrawingSystem::GenerateLineCoordinates(const FHexCoordinate& Start, const FHexCoordinate& Target) const
{
	// Use the coordinate utility function for pathfinding
	TArray<FHexCoordinate> Path = FHexCoordinateUtils::GetPathBetween(Start, Target);

	// Limit to maximum distance if set
	if (MaxLineDrawDistance > 0 && Path.Num() > MaxLineDrawDistance)
	{
		Path.SetNum(MaxLineDrawDistance);
	}

	return Path;
}

TArray<FHexCoordinate> UHexDrawingSystem::GenerateFillCoordinates(const FHexCoordinate& Center, int32 MaxDepth) const
{
	return FHexCoordinateUtils::GetCoordinatesInRadius(Center, MaxDepth);
}

void UHexDrawingSystem::CreatePreviewVertices(const TArray<FHexCoordinate>& Coordinates)
{
	if (!OwningHexGraph.Get() || !PreviewVertexClass)
	{
		return;
	}

	UWorld* World = OwningHexGraph->GetWorld();
	if (!World)
	{
		return;
	}

	for (const FHexCoordinate& Coordinate : Coordinates)
	{
		// Don't create preview where permanent vertices already exist
		if (VertexManager.Get() && VertexManager->GetVertex(Coordinate, false))
		{
			continue;
		}

		FVector WorldPosition = GetWorldPositionForCoordinate(Coordinate);
		FTransform SpawnTransform(FRotator::ZeroRotator, WorldPosition, FVector::OneVector);

		AVertex* PreviewVertex = World->SpawnActor<AVertex>(
			PreviewVertexClass,
			SpawnTransform,
			FActorSpawnParameters()
		);

		if (PreviewVertex)
		{
			PreviewVertex->row = Coordinate.Row;
			PreviewVertex->col = Coordinate.Col;
			PreviewVertices.Add(PreviewVertex);
			LastPreviewVertex = PreviewVertex;
		}
	}
}

void UHexDrawingSystem::DestroyPreviewVertices()
{
	for (TObjectPtr<AVertex> PreviewVertex : PreviewVertices)
	{
		if (IsValid(PreviewVertex.Get()))
		{
			PreviewVertex->Destroy();
		}
	}

	PreviewVertices.Empty();
	LastPreviewVertex = nullptr;
}

void UHexDrawingSystem::SetDrawingState(EHexDrawingState NewState)
{
	if (CurrentDrawingState != NewState)
	{
		EHexDrawingState PreviousState = CurrentDrawingState;
		CurrentDrawingState = NewState;

		UE_LOG(LogHexGraph, VeryVerbose, TEXT("HexDrawingSystem: State changed from %d to %d"), 
			   static_cast<int32>(PreviousState), static_cast<int32>(NewState));
	}
}

TArray<FHexCoordinate> UHexDrawingSystem::ValidateDrawingCoordinates(const TArray<FHexCoordinate>& Coordinates) const
{
	TArray<FHexCoordinate> ValidCoordinates;

	for (const FHexCoordinate& Coordinate : Coordinates)
	{
		if (Coordinate.IsValid())
		{
			ValidCoordinates.Add(Coordinate);
		}
	}

	return ValidCoordinates;
}

FVector UHexDrawingSystem::GetWorldPositionForCoordinate(const FHexCoordinate& Coordinate) const
{
	if (!OwningHexGraph.Get())
	{
		return FVector::ZeroVector;
	}

	return FHexCoordinateUtils::CoordinateToWorldPosition(
		Coordinate, 
		OwningHexGraph->MeshLength, 
		OwningHexGraph->VertexSpacing
	);
}

void UHexDrawingSystem::LoadDrawingSettings()
{
	if (!OwningHexGraph.Get())
	{
		return;
	}

	const UHexGraphSettings* Settings = UHexGraphSettings::GetHexGraphSettings();
	if (Settings)
	{
		MaxLineDrawDistance = Settings->MaxFillDepth * 2; // Use fill depth as basis for line draw distance
	}

	// Load preview vertex class from HexGraph
	PreviewVertexClass = OwningHexGraph->previewVertexClass;

	UE_LOG(LogHexGraph, Log, TEXT("HexDrawingSystem: Loaded settings - MaxDistance: %d"), MaxLineDrawDistance);
}