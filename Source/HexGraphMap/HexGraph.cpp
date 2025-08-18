// Fill out your copyright notice in the Description page of Project Settings.


#include "HexGraph.h"
#include "HexGraphMap.h"
#include "HexGraphValidation.h"
#include "HexGraphSettings.h"
#include "EnhancedInputComponent.h"
#include <EnhancedInputSubsystems.h>

int AHexGraph::verticesCreated = 0;

// Sets default values
AHexGraph::AHexGraph()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	vertices = TMap<FString, AVertex*>();
	adjacencyMatrix = TMap<FString, UAdjacencyMap*>();

	tempVertices = TMap<FString, AVertex*>();
	tempAdjacencyMatrix = TMap<FString, UAdjacencyMap*>();
	previewVertices = TArray<AVertex*>();

	// Initialize configuration from settings
	const UHexGraphSettings* Settings = UHexGraphSettings::GetHexGraphSettings();
	
	MeshLength = Settings->DefaultMeshLength;
	VertexSpacing = Settings->DefaultVertexSpacing;
	PanSpeed = Settings->DefaultPanSpeed;
	ZoomPercent = Settings->DefaultZoomPercent;
	maxFillDepth = Settings->MaxFillDepth;
	useMouseFollower = Settings->bUseMouseFollower;
	
	lineDrawActivated = false;
	pieceSelected = false;

	StaticMeshComp = CreateDefaultSubobject <UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));

	StaticMeshComp->SetupAttachment(RootComponent);
	SpringArmComp->SetupAttachment(StaticMeshComp);
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);

	SpringArmComp->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 400.f), FRotator(-90.f, 0.0f, 0.0f));
	SpringArmComp->TargetArmLength = 400.f;
	SpringArmComp->bEnableCameraLag = true;
	SpringArmComp->CameraLagSpeed = 3.0f;

	// Initialize Manager System
	InitializeManagers();
}

AGraphVertex* AHexGraph::AddFirstVertex()
{
	// Use VertexManager if available, otherwise fall back to legacy method
	if (VertexManager)
	{
		// Create the first vertex using VertexManager
		FHexCoordinate StartCoord(0, 0);
		TArray<FString> Adjacencies = { "1:0", "1:1", "0:1", "-1:0", "0:-1", "1:-1" };
		
		AVertex* firstVertex = VertexManager->CreateVertexWithAdjacencies(
			defaultVertexClass, 
			StartCoord, 
			FTransform(), 
			Adjacencies, 
			false
		);

		AGraphVertex* graphVertex = Cast<AGraphVertex>(firstVertex);
		if (!graphVertex)
		{
			UE_LOG(LogHexGraph, Error, TEXT("AddFirstVertex: Failed to create graph vertex with VertexManager"));
			return nullptr;
		}

		//Set the MeshLength of the vertices
		FTransform vertexTransform = graphVertex->GetTransform();
		FBoxSphereBounds vertexBounds = graphVertex->MeshComponent->CalcBounds(vertexTransform);
		MeshLength = vertexBounds.BoxExtent.X * 2.0f;

		//Build PlaceHolderVertices for all 6 directions using VertexManager
		EHexagonDirection firstNeighborDirection = EHexagonDirection::Southeast;
		EHexagonDirection secondNeighborDirection = EHexagonDirection::South;
		EHexagonDirection thirdNeighborDirection = EHexagonDirection::Southwest;
		for (int i = 0; i < 6; i++)
		{
			EHexagonDirection Direction = static_cast<EHexagonDirection>(i);
			AVertex* newPlaceHolderVert = VertexManager->CreateVertexInDirection(
				placeHolderVertexClass, 
				graphVertex, 
				Direction, 
				false
			);
			
			if (newPlaceHolderVert)
			{
				// Set up adjacencies for the placeholder (match legacy method)
				TArray<FString> placeHolderAdjacencies = { "", "", "", "", "", "" };

				placeHolderAdjacencies[static_cast<int32>(firstNeighborDirection)] = GetCoordInDirection(newPlaceHolderVert, firstNeighborDirection);
				placeHolderAdjacencies[static_cast<int32>(secondNeighborDirection)] = GetCoordInDirection(newPlaceHolderVert, secondNeighborDirection);
				placeHolderAdjacencies[static_cast<int32>(thirdNeighborDirection)] = GetCoordInDirection(newPlaceHolderVert, thirdNeighborDirection);

				UAdjacencyMap* NewPlaceHolderVertAdjMap = GetAdjacenciesForVertex(newPlaceHolderVert);
				if (NewPlaceHolderVertAdjMap)
				{
					NewPlaceHolderVertAdjMap->setAllAdjacencies(placeHolderAdjacencies);
				}
			}
			
			firstNeighborDirection = static_cast<EHexagonDirection>((static_cast<int32>(firstNeighborDirection) + 1) % 6);
			secondNeighborDirection = static_cast<EHexagonDirection>((static_cast<int32>(secondNeighborDirection) + 1) % 6);
			thirdNeighborDirection = static_cast<EHexagonDirection>((static_cast<int32>(thirdNeighborDirection) + 1) % 6);
		}

		return graphVertex;
	}
	else
	{
		// Fallback to legacy method
		//Spawn the first vertex actor for the graph
		AGraphVertex* firstVertex = Cast<AGraphVertex>(AddVertexWithAdjacencies(defaultVertexClass, 0, 0, FTransform(), { "1:0", "1:1", "0:1", "-1:0", "0:-1", "1:-1" }, false));

		//TODO: Add verification that the X and Y directions of all possibleVertices are consistent

		//Set the MeshLength of the vertices
		FTransform vertexTransform = firstVertex->GetTransform();
		FBoxSphereBounds vertexBounds = firstVertex->MeshComponent->CalcBounds(vertexTransform);
		MeshLength = vertexBounds.BoxExtent.X * 2.0f;

		//Build PlaceHolderVertices for all 6 directions
		EHexagonDirection firstNeighborDirection = EHexagonDirection::Southeast;
		EHexagonDirection secondNeighborDirection = EHexagonDirection::South;
		EHexagonDirection thirdNeighborDirection = EHexagonDirection::Southwest;
		for (int i = 0; i < 6; i++)
		{
			AVertex* newPlaceHolderVert = AddVertexInDirection(placeHolderVertexClass, firstVertex, static_cast<EHexagonDirection>(i));
			
			TArray<FString> placeHolderAdjacencies = { "", "", "", "", "", "" };

			placeHolderAdjacencies[static_cast<int32>(firstNeighborDirection)] = GetCoordInDirection(newPlaceHolderVert, firstNeighborDirection);
			placeHolderAdjacencies[static_cast<int32>(secondNeighborDirection)] = GetCoordInDirection(newPlaceHolderVert, secondNeighborDirection);
			placeHolderAdjacencies[static_cast<int32>(thirdNeighborDirection)] = GetCoordInDirection(newPlaceHolderVert, thirdNeighborDirection);

			UAdjacencyMap* NewPlaceHolderVertAdjMap2 = GetAdjacenciesForVertex(newPlaceHolderVert);
			if (NewPlaceHolderVertAdjMap2)
			{
				NewPlaceHolderVertAdjMap2->setAllAdjacencies(placeHolderAdjacencies);
			}

			firstNeighborDirection = static_cast<EHexagonDirection>((static_cast<int32>(firstNeighborDirection) + 1) % 6);
			secondNeighborDirection = static_cast<EHexagonDirection>((static_cast<int32>(secondNeighborDirection) + 1) % 6);
			thirdNeighborDirection = static_cast<EHexagonDirection>((static_cast<int32>(thirdNeighborDirection) + 1) % 6);
		}

		return firstVertex;
	}
}

AVertex* AHexGraph::AddVertexByCoord(TSubclassOf<AVertex> vertexClass, FString coord, FTransform spawnTransform, bool isTemp)
{
	int row, col;
	coordStringToInts(coord, row, col);
	return AddVertexByRowCol(vertexClass, row, col, spawnTransform, isTemp);
}

//Adds a vertex to the map that has no adjacencies
AVertex* AHexGraph::AddVertexByRowCol(TSubclassOf<AVertex> vertexClass, int row, int col, FTransform spawnTransform, bool isTemp)
{
	FActorSpawnParameters spawnParams = BuildVertexSpawnParams();

	//Spawn the new vertex actor
	AVertex* newVertex = GetWorld()->SpawnActor<AVertex>(vertexClass, spawnTransform, spawnParams);
	
	//Initialize the vertex
	InitializeVertex(newVertex, row, col, isTemp);
	
	return newVertex;
}

AVertex* AHexGraph::AddVertexInDirection(TSubclassOf<AVertex> vertexClass, AVertex* rootVertex, EHexagonDirection direction, bool isTemp)
{
	// Calculate the center point of the placeholder
	FVector2D directionUnitVector = GetUnitVectorInHexDirection(static_cast<int32>(direction));
	FTransform newTransform = rootVertex->GetActorTransform();
	const FVector2D& movementVector = (directionUnitVector * MeshLength) + (directionUnitVector * VertexSpacing);
	newTransform.AddToTranslation(FVector(movementVector.X, movementVector.Y, 0.0f));

	//Get coordinate and add vertex
	FString coord = GetCoordInDirection(rootVertex, direction);
	AVertex* newVertex = AddVertexByCoord(vertexClass, coord, newTransform, isTemp);

	return newVertex;
}

// Called when the game starts or when spawned
void AHexGraph::BeginPlay()
{
	Super::BeginPlay();

	// Setup manager system
	SetupManagerEventBindings();

	InitializeHUD();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(defaultInputMapping.LoadSynchronous(), 0);
		}
	}

	AddFirstVertex();

	if (useMouseFollower) {
		mouseFollower = GetWorld()->SpawnActor<AActor>(mouseFollowerClass, FActorSpawnParameters());
	}
}

void AHexGraph::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	// Clean up invalid vertex references
	int32 CleanedVertices = CleanupInvalidVertexReferences();
	
	// Clean up managed adjacency maps to prevent memory leaks
	CleanupManagedAdjacencyMaps();
	
	UE_LOG(LogHexGraph, Log, TEXT("EndPlay: HexGraph cleanup completed (cleaned %d invalid vertex references)"), CleanedVertices);
}

FActorSpawnParameters AHexGraph::BuildVertexSpawnParams()
{
	FActorSpawnParameters params = FActorSpawnParameters();
	params.Name = FName(FString::Printf(TEXT("Vertex-%d"), verticesCreated++));
	params.Owner = this;

	return params;
}

AVertex* AHexGraph::GetVertex(FString coord)
{
	// Validate coordinate string format
	HEXGRAPH_VALIDATE_COORD_STRING(coord, nullptr);
	
	// Check if vertex exists in the map
	if (!vertices.Contains(coord))
	{
		UE_LOG(LogHexGraph, VeryVerbose, TEXT("GetVertex: No vertex found at coordinate '%s'"), *coord);
		return nullptr;
	}
	
	// Get vertex pointer and validate it
	AVertex** VertexPtr = vertices.Find(coord);
	if (!VertexPtr || !IsValid(*VertexPtr))
	{
		UE_LOG(LogHexGraph, Error, TEXT("GetVertex: Invalid vertex pointer at coordinate '%s'"), *coord);
		// Clean up invalid entry
		vertices.Remove(coord);
		return nullptr;
	}
	
	return *VertexPtr;
}

void AHexGraph::RemoveVertexAtCoord(FString coord)
{
	if (AVertex* removedVertex = GetVertex(coord)) 
	{
		UAdjacencyMap* removedVertexAdjacencies = *adjacencyMatrix.Find(coord);
		TArray<FString> neighbors = removedVertexAdjacencies->adjacentVertexCoords;
		for (int i = 0; i < neighbors.Num(); i++)
		{
			//Remove coord from all neighbor adjancencies
			FString neighborCoord = neighbors[i];
			if (neighborCoord != "") {

				if (AVertex* neighborVert = GetVertex(neighborCoord))
				{
					//Remove all placeholders with only placeholder neighbors if removed vert is a graph vert
					if (removedVertex->type == EVertexType::Graph)
					{
						//Check if current neighbor is a placeholder
						if (neighborVert->type == EVertexType::PlaceHolder) {

							//Check for any Graph neighbors of the placeholder
							UAdjacencyMap* NeighborVertAdjMap = GetAdjacenciesForVertex(neighborVert);
							if (!NeighborVertAdjMap)
							{
								continue; // Skip if no adjacency map
							}
							TArray<FString> phNeighbors = NeighborVertAdjMap->adjacentVertexCoords;
							bool noGraphNeighbor = true;
							for (int j = 0; j < phNeighbors.Num() && noGraphNeighbor; j++)
							{
								FString phNeighborCoord = phNeighbors[j];
								if (phNeighborCoord != coord)
								{
									if (AVertex* ph = GetVertex(phNeighborCoord)) {
										noGraphNeighbor = !(ph->type == EVertexType::Graph);
									}
								}
							}

							//Remove the placeholder if it has no other graph neighbors
							if (noGraphNeighbor)
							{
								RemoveVertexAtCoord(neighborCoord);
							}
						}
					}

					//if (!(removedVertex->type == EVertexType::Graph && neighborVert->type == EVertexType::Graph))
					//Remove all references to removedVertex from the neighbor except when both are Graph vertices
					else if (removedVertex->type == EVertexType::PlaceHolder)
					{
						UAdjacencyMap* NeighborVertAdjMap2 = GetAdjacenciesForVertex(neighborVert);
						if (!NeighborVertAdjMap2)
						{
							continue; // Skip if no adjacency map
						}
						TArray<FString> neighborAdjacencies = NeighborVertAdjMap2->adjacentVertexCoords;
						for (FString& adjacencyCoord : neighborAdjacencies) {
							if (adjacencyCoord == coord) {
								adjacencyCoord = "";
							}
						}
						NeighborVertAdjMap2->setAllAdjacencies(neighborAdjacencies);
					}
				}
			}


		}

		//If removing a GraphVertex, demote it to a placeholder instead
		bool demoted = false;
		if (removedVertex->type == EVertexType::Graph)
		{
			//Only demote if the resulting placeholder would still have Graph neighbors
			bool noNeighbors = true;
			neighbors = removedVertexAdjacencies->adjacentVertexCoords;
			for (int i = 0; i < neighbors.Num() && noNeighbors; i++)
			{
				FString neighborCoord = neighbors[i];
				if (AVertex* neighborVert = GetVertex(neighborCoord))
				{
					noNeighbors = !(neighborVert->type == EVertexType::Graph);
				}
			}

			if (!noNeighbors)
			{
				removedVertex = DemoteInstanceToPlaceHolder(Cast<AGraphVertex>(removedVertex));
				demoted = true;
			}
		}


		if (!demoted) {
			vertices.Remove(coord);
			adjacencyMatrix.Remove(coord);
			removedVertex->Destroy();
		}

		//Clear previousSelectedVertex if it is the removed vertex
		if (previousVertexSelection && coord == previousVertexSelection->Coord())
		{
			previousVertexSelection = nullptr;
		}
	}
}

UAdjacencyMap* AHexGraph::GetAdjacenciesForVertex(AVertex* vertex)
{
	// Validate vertex pointer
	HEXGRAPH_VALIDATE_VERTEX(vertex, nullptr);
	
	// Get coordinate and validate it
	FString coord = vertex->Coord();
	HEXGRAPH_VALIDATE_COORD_STRING(coord, nullptr);
	
	// Check if adjacency map exists
	if (!adjacencyMatrix.Contains(coord))
	{
		UE_LOG(LogHexGraph, Error, TEXT("GetAdjacenciesForVertex: No adjacency map found for vertex at '%s'"), *coord);
		return nullptr;
	}
	
	// Get adjacency map and validate it
	UAdjacencyMap** AdjMapPtr = adjacencyMatrix.Find(coord);
	if (!AdjMapPtr || !IsValid(*AdjMapPtr))
	{
		UE_LOG(LogHexGraph, Error, TEXT("GetAdjacenciesForVertex: Invalid adjacency map at '%s'"), *coord);
		// Clean up invalid entry
		adjacencyMatrix.Remove(coord);
		return nullptr;
	}
	
	return *AdjMapPtr;
}

UAdjacencyMap* AHexGraph::CreateManagedAdjacencyMap()
{
	// Create new adjacency map with this HexGraph as owner
	UAdjacencyMap* NewAdjMap = NewObject<UAdjacencyMap>(this);
	
	if (!IsValid(NewAdjMap))
	{
		UE_LOG(LogHexGraph, Error, TEXT("CreateManagedAdjacencyMap: Failed to create adjacency map"));
		return nullptr;
	}
	
	// Add to managed tracking array for automatic cleanup
	ManagedAdjacencyMaps.Add(NewAdjMap);
	
	UE_LOG(LogHexGraph, VeryVerbose, TEXT("CreateManagedAdjacencyMap: Created and tracked adjacency map (Total managed: %d)"), 
		   ManagedAdjacencyMaps.Num());
	
	return NewAdjMap;
}

void AHexGraph::CleanupManagedAdjacencyMaps()
{
	UE_LOG(LogHexGraph, Log, TEXT("CleanupManagedAdjacencyMaps: Cleaning up %d managed adjacency maps"), 
		   ManagedAdjacencyMaps.Num());
	
	// Remove any invalid objects from the managed array
	int32 InitialCount = ManagedAdjacencyMaps.Num();
	ManagedAdjacencyMaps.RemoveAll([](const TObjectPtr<UAdjacencyMap>& AdjMap) {
		return !IsValid(AdjMap.Get());
	});
	
	int32 RemovedCount = InitialCount - ManagedAdjacencyMaps.Num();
	if (RemovedCount > 0)
	{
		UE_LOG(LogHexGraph, Log, TEXT("CleanupManagedAdjacencyMaps: Removed %d invalid adjacency maps"), RemovedCount);
	}
	
	// Clear all remaining references (they will be garbage collected)
	ManagedAdjacencyMaps.Empty();
	
	UE_LOG(LogHexGraph, Log, TEXT("CleanupManagedAdjacencyMaps: Cleanup complete"));
}

void AHexGraph::ValidateAllVertexReferences()
{
	UE_LOG(LogHexGraph, Log, TEXT("ValidateAllVertexReferences: Starting validation of all vertex references"));
	
	int32 InvalidReferences = 0;
	int32 TotalReferences = 0;
	
	// Check main vertices map
	for (auto& VertexPair : vertices)
	{
		TotalReferences++;
		if (!IsValid(VertexPair.Value))
		{
			InvalidReferences++;
			UE_LOG(LogHexGraph, Warning, TEXT("ValidateAllVertexReferences: Invalid vertex at '%s'"), *VertexPair.Key);
		}
	}
	
	// Check temp vertices map
	for (auto& VertexPair : tempVertices)
	{
		TotalReferences++;
		if (!IsValid(VertexPair.Value))
		{
			InvalidReferences++;
			UE_LOG(LogHexGraph, Warning, TEXT("ValidateAllVertexReferences: Invalid temp vertex at '%s'"), *VertexPair.Key);
		}
	}
	
	// Check special references
	if (hoverTarget && !IsValid(hoverTarget))
	{
		InvalidReferences++;
		UE_LOG(LogHexGraph, Warning, TEXT("ValidateAllVertexReferences: Invalid hoverTarget reference"));
	}
	
	if (previousVertexSelection && !IsValid(previousVertexSelection))
	{
		InvalidReferences++;
		UE_LOG(LogHexGraph, Warning, TEXT("ValidateAllVertexReferences: Invalid previousVertexSelection reference"));
	}
	
	UE_LOG(LogHexGraph, Log, TEXT("ValidateAllVertexReferences: Found %d invalid references out of %d total"), 
		   InvalidReferences, TotalReferences);
}

int32 AHexGraph::CleanupInvalidVertexReferences()
{
	UE_LOG(LogHexGraph, Log, TEXT("CleanupInvalidVertexReferences: Starting cleanup of invalid vertex references"));
	
	int32 CleanedReferences = 0;
	
	// Clean up main vertices map
	TArray<FString> InvalidKeys;
	for (auto& VertexPair : vertices)
	{
		if (!IsValid(VertexPair.Value))
		{
			InvalidKeys.Add(VertexPair.Key);
		}
	}
	
	for (const FString& Key : InvalidKeys)
	{
		vertices.Remove(Key);
		adjacencyMatrix.Remove(Key); // Also remove corresponding adjacency map
		CleanedReferences++;
		UE_LOG(LogHexGraph, Log, TEXT("CleanupInvalidVertexReferences: Removed invalid vertex at '%s'"), *Key);
	}
	
	// Clean up temp vertices map
	InvalidKeys.Empty();
	for (auto& VertexPair : tempVertices)
	{
		if (!IsValid(VertexPair.Value))
		{
			InvalidKeys.Add(VertexPair.Key);
		}
	}
	
	for (const FString& Key : InvalidKeys)
	{
		tempVertices.Remove(Key);
		tempAdjacencyMatrix.Remove(Key); // Also remove corresponding adjacency map
		CleanedReferences++;
		UE_LOG(LogHexGraph, Log, TEXT("CleanupInvalidVertexReferences: Removed invalid temp vertex at '%s'"), *Key);
	}
	
	// Clean up special references
	if (hoverTarget && !IsValid(hoverTarget))
	{
		hoverTarget = nullptr;
		CleanedReferences++;
		UE_LOG(LogHexGraph, Log, TEXT("CleanupInvalidVertexReferences: Cleared invalid hoverTarget"));
	}
	
	if (previousVertexSelection && !IsValid(previousVertexSelection))
	{
		previousVertexSelection = nullptr;
		CleanedReferences++;
		UE_LOG(LogHexGraph, Log, TEXT("CleanupInvalidVertexReferences: Cleared invalid previousVertexSelection"));
	}
	
	UE_LOG(LogHexGraph, Log, TEXT("CleanupInvalidVertexReferences: Cleaned up %d invalid references"), CleanedReferences);
	return CleanedReferences;
}

void AHexGraph::RefreshSettingsValues()
{
	const UHexGraphSettings* Settings = UHexGraphSettings::GetHexGraphSettings();
	if (!Settings)
	{
		UE_LOG(LogHexGraph, Warning, TEXT("RefreshSettingsValues: Could not get HexGraphSettings, using defaults"));
		return;
	}

	// Update runtime values from settings
	VertexSpacing = Settings->DefaultVertexSpacing;
	PanSpeed = Settings->DefaultPanSpeed;
	ZoomPercent = Settings->DefaultZoomPercent;
	maxFillDepth = Settings->MaxFillDepth;
	useMouseFollower = Settings->bUseMouseFollower;
	
	// Note: MeshLength is calculated dynamically and shouldn't be overridden
	
	UE_LOG(LogHexGraph, Log, TEXT("RefreshSettingsValues: Updated settings - VertexSpacing: %f, PanSpeed: %f, ZoomPercent: %f, MaxFillDepth: %d"), 
		   VertexSpacing, PanSpeed, ZoomPercent, maxFillDepth);
}

void AHexGraph::RefreshSettings()
{
	RefreshSettingsValues();
	UE_LOG(LogHexGraph, Log, TEXT("RefreshSettings: Manual settings refresh completed via console command"));
}

//Adds a vertex to the map and sets its adjacencies to the given list; adds adjacencies for all neighbors if forceBiDirectionalAdjacency is set to true
AVertex* AHexGraph::AddVertexWithAdjacencies(TSubclassOf<AVertex> vertexClass, int row, int col, FTransform spawnTransform, TArray<FString> adjacencies, bool isTemp)
{
	AVertex* newVertex = AddVertexByRowCol(vertexClass, row, col, spawnTransform);
	UAdjacencyMap* NewVertexAdjMap = GetAdjacenciesForVertex(newVertex);
	if (NewVertexAdjMap)
	{
		NewVertexAdjMap->setAllAdjacencies(adjacencies);
	}

	return newVertex;
}

void AHexGraph::HandleVertexRightClick(AVertex* clickedVertex)
{
	UE_LOG(LogHexGraph, Log, TEXT("Adjacencies for clicked Vertex:\n %s"), *(*adjacencyMatrix.Find(clickedVertex->Coord()))->adjacencyString());
}

AVertex* AHexGraph::PromotePlaceholderToInstance(APlaceHolderVertex* placeHolderVertex)
{
	UAdjacencyMap* oldAdjacencies = GetAdjacenciesForVertex(placeHolderVertex);
	AGraphVertex* promotedVertex = GetWorld()->SpawnActor<AGraphVertex>(defaultVertexClass, placeHolderVertex->GetActorTransform());
	InitializeVertex(promotedVertex, placeHolderVertex->row, placeHolderVertex->col);
	vertices[placeHolderVertex->Coord()] = promotedVertex;
	adjacencyMatrix[placeHolderVertex->Coord()] = oldAdjacencies;

	TArray<FString> neighbors = oldAdjacencies->adjacentVertexCoords;
	placeHolderVertex->Destroy();

	//Build new placeholders in all empty neighbor coordinates
	TArray<FString> newPlaceHolders = {};
	for (int i = 0; i < neighbors.Num(); i++)
	{
		if (neighbors[i] == "") {
			AVertex* newPlaceHolder = AddVertexInDirection(placeHolderVertexClass, promotedVertex, static_cast<EHexagonDirection>(i));

			//Check all directions for adjacencies
			TArray<FString> placeHolderAdjacencies = { "", "", "", "", "", "" };
			for (int j = 0; j < 6; j++)
			{
				FString coord = GetCoordInDirection(newPlaceHolder, static_cast<EHexagonDirection>(j));

				//Ensure bidirectional adjacency with all other vertices
				if (vertices.Contains(coord)) {
					placeHolderAdjacencies[j] = coord;
					UAdjacencyMap* ExistingVertexAdjMap = GetAdjacenciesForVertex(GetVertex(coord));
					if (ExistingVertexAdjMap)
					{
						ExistingVertexAdjMap->setDirectionsAdjacency((j + 3)%6, newPlaceHolder->Coord());
					}
				}
			}
			UAdjacencyMap* NewPlaceHolderAdjMap = GetAdjacenciesForVertex(newPlaceHolder);
			if (NewPlaceHolderAdjMap)
			{
				NewPlaceHolderAdjMap->setAllAdjacencies(placeHolderAdjacencies);
			}
			UAdjacencyMap* PromotedVertexAdjMap = GetAdjacenciesForVertex(promotedVertex);
			if (PromotedVertexAdjMap)
			{
				PromotedVertexAdjMap->setDirectionsAdjacency(i, newPlaceHolder->Coord());
			}

			newPlaceHolders.Add(newPlaceHolder->Coord());
		}
	}

	return promotedVertex;
}

FString AHexGraph::GetCoordInDirection(AVertex* vert, EHexagonDirection direction)
{
	int row, col;
	coordStringToInts(vert->Coord(), row, col);

	switch (direction)
	{
	case EHexagonDirection::North:
		row += 1;
		break;
	case EHexagonDirection::Northeast:
		if (col % 2 == 0) {
			row += 1;
			col += 1;
		}
		else {
			col += 1;
		}
		break;
	case EHexagonDirection::Southeast:
		if (col % 2 != 0) {
			row -= 1;
			col += 1;
		}
		else {
			col += 1;
		}
		break;
	case EHexagonDirection::South:
		row -= 1;
		break;
	case EHexagonDirection::Southwest:
		if (col % 2 != 0) {
			row -= 1;
			col -= 1;
		}
		else {
			col -= 1;
		}
		break;
	case EHexagonDirection::Northwest:
		if (col % 2 == 0) {
			row += 1;
			col -= 1;
		}
		else {
			col -= 1;
		}
		break;
	default:
		break;
	}

	return intsToCoordString(row, col);
}

FString AHexGraph::GetCoordFromMousePosition()
{
	FVector mousePosition = ProjectMousePositionToActorXY(this);
	float rowHalfStep = (MeshLength + VertexSpacing) / 2;
	FVector2D neUnitVector = GetUnitVectorInHexDirection(1);
	float colHalfStep = ((neUnitVector * MeshLength) + (neUnitVector * VertexSpacing)).Y / 2;


	int colHalfStepsToMouse;
	if (mousePosition.Y >= 0) {
		colHalfStepsToMouse = FMath::CeilToInt(mousePosition.Y / colHalfStep);
	}
	else {
		colHalfStepsToMouse = FMath::FloorToInt(mousePosition.Y / colHalfStep);
	}
	int colForMouse = colHalfStepsToMouse / 2;

	float rowDistance = mousePosition.X;
	if (FMath::Abs(colForMouse % 2) == 1) {
		rowDistance += rowHalfStep;
	}

	int rowHalfStepsToMouse;
	if (mousePosition.X >= 0) {
		rowHalfStepsToMouse = FMath::CeilToInt(rowDistance / rowHalfStep);
	}
	else {
		rowHalfStepsToMouse = FMath::FloorToInt(rowDistance / rowHalfStep);
	}
	int rowForMouse = rowHalfStepsToMouse / 2;
	return FString::Printf(TEXT("%d:%d"), rowForMouse, colForMouse);
}

void AHexGraph::ListConnectedVertices(TArray<AVertex*>& connected, AVertex* startingVertex, const TSet<EVertexType>& includedTypes)
{
	connected.Add(startingVertex);

	for (int i = 0; i < 6; i++)
	{
		EHexagonDirection currentDirection = static_cast<EHexagonDirection>(i);
		FString neighborCoord = GetCoordInDirection(startingVertex, currentDirection);
		if (AVertex* neighbor = GetVertex(neighborCoord))
		{
			//If there is no filtering or the neighbor type is in the filter AND the neighbor isnt in the list 
			// already recurse on it
			if ((includedTypes.IsEmpty() || includedTypes.Contains(neighbor->type)) && !connected.Contains(neighbor))
			{
				ListConnectedVertices(connected, neighbor, includedTypes);
			}
		}
	}

}

void AHexGraph::SelectGraphPiece(TMap<FString, TSubclassOf<AVertex>> pieceVertices, TMap<FString, FString> stringifiedAdjacencyMap, FString pieceName)
{
	pieceSelected = true;
	selectedPieceVertexClasses = pieceVertices;
	selectedPieceAdjacencyMatrix = TMap<FString, UAdjacencyMap*>();

	for (const TPair<FString, FString>& coordAdjacencyPair : stringifiedAdjacencyMap) 
	{
		UAdjacencyMap* currentMap = CreateManagedAdjacencyMap();
		if (!currentMap)
		{
			UE_LOG(LogHexGraph, Error, TEXT("SelectGraphPiece: Failed to create managed adjacency map for piece '%s'"), *pieceName);
			continue;
		}
		
		TArray<FString> currentAdj;
		coordAdjacencyPair.Value.ParseIntoArray(currentAdj, TEXT(";"), false);
		currentMap->setAllAdjacencies(currentAdj);

		selectedPieceAdjacencyMatrix.Add(coordAdjacencyPair.Key, currentMap);
	}
}

void AHexGraph::InitializeVertex(AVertex* vertex, int row, int col, bool isTemp, UAdjacencyMap* adjacencyMap)
{
	// Validate vertex pointer
	HEXGRAPH_VALIDATE_VERTEX(vertex, );
	
	// Validate coordinates
	if (!UHexGraphValidation::IsValidCoordinate(row, col))
	{
		UE_LOG(LogHexGraph, Error, TEXT("InitializeVertex: Invalid coordinates (%d, %d)"), row, col);
		return;
	}
	
	// Set vertex coordinates
	vertex->row = row;
	vertex->col = col;
	
	// Get coordinate string for map operations
	FString coord = vertex->Coord();
	
	// Validate or create adjacency map using managed system
	UAdjacencyMap* adjMap = adjacencyMap;
	if (!adjMap)
	{
		adjMap = CreateManagedAdjacencyMap();
		if (!adjMap)
		{
			UE_LOG(LogHexGraph, Error, TEXT("InitializeVertex: Failed to create managed adjacency map for vertex at '%s'"), *coord);
			return;
		}
	}

	// Add to appropriate storage maps
	if (isTemp) {
		tempVertices.Add(coord, vertex);
		tempAdjacencyMatrix.Add(coord, adjMap);
		UE_LOG(LogHexGraph, VeryVerbose, TEXT("InitializeVertex: Added temporary vertex at '%s'"), *coord);
	}
	else {
		vertices.Add(coord, vertex);
		adjacencyMatrix.Add(coord, adjMap);
		UE_LOG(LogHexGraph, VeryVerbose, TEXT("InitializeVertex: Added permanent vertex at '%s'"), *coord);
	}

	// Set up event handlers
	vertex->OnBeginCursorOver.AddDynamic(this, &AHexGraph::OnVertexHoverBegin);
	vertex->OnEndCursorOver.AddDynamic(this, &AHexGraph::OnVertexHoverEnd);
}

void AHexGraph::OnSelect()
{
	if (lineDrawActivated) {
		CommitTempToGraph();
		lineDrawActivated = false;
	}
	else if (hoverTarget) {
		HandleVertexLeftClick(hoverTarget);
		previousVertexSelection = hoverTarget;
		hoverTarget = nullptr;
	}
}

APlaceHolderVertex* AHexGraph::DemoteInstanceToPlaceHolder(AGraphVertex* graphVertex)
{
	UAdjacencyMap* oldAdjacencies = GetAdjacenciesForVertex(graphVertex);
	APlaceHolderVertex* demotedVertex = GetWorld()->SpawnActor<APlaceHolderVertex>(placeHolderVertexClass, graphVertex->GetActorTransform());
	InitializeVertex(demotedVertex, graphVertex->row, graphVertex->col);
	vertices[graphVertex->Coord()] = demotedVertex;
	adjacencyMatrix[graphVertex->Coord()] = oldAdjacencies;
	graphVertex->Destroy();
	return demotedVertex;
}

void AHexGraph::OnDelete() 
{
	UE_LOG(LogHexGraph, Log, TEXT("\n\nOnDelete START\n\n"));
	if (!deleting && hoverTarget && hoverTarget->type == EVertexType::Graph) {
		deleting = true;
		RemoveVertexAtCoord(hoverTarget->Coord());
		hoverTarget = nullptr;
		deleting = false;
	}
}

void AHexGraph::OnStartLineDraw()
{
	UE_LOG(LogHexGraph, Log, TEXT("\n\nLINE DRAW START\n\n"));
	lineDrawActivated = true;
}

void AHexGraph::OnStopLineDraw()
{
	UE_LOG(LogHexGraph, Log, TEXT("\n\nLINE DRAW END\n\n"));
	lineDrawActivated = false;

	ClearLineDraw();
}

void AHexGraph::ClearLineDraw() 
{
	//Destroy all created previews and refresh the previews array
	for (AVertex* previewVertex : previewVertices)
	{
		previewVertex->Destroy();
	}
	previewVertices = TArray<AVertex*>();
	tempVertices = TMap<FString, AVertex*>();
	tempAdjacencyMatrix = TMap<FString, UAdjacencyMap*>();
	hoverTarget = nullptr;
	lastPreview = nullptr;
}

void AHexGraph::OnFill()
{
	if (hoverTarget && hoverTarget->type == EVertexType::PlaceHolder) 
	{
		FillConnections(hoverTarget);
	}
	FString mouseCoord = GetCoordFromMousePosition();

	UE_LOG(LogHexGraph, Log, TEXT("Coord: %s"), *mouseCoord);
}

void AHexGraph::FillConnections(AVertex* vert, int depth)
{
	//List all PHverts connected to vert
	TArray<AVertex*> connectedPHs;
	ListConnectedVertices(connectedPHs, vert, TSet<EVertexType>({ EVertexType::PlaceHolder }));

	//Promote all connected placeholders
	for (int i = 0; i < connectedPHs.Num(); i++)
	{
		APlaceHolderVertex* vertToPromote = Cast<APlaceHolderVertex>(connectedPHs[i]);
		connectedPHs[i] = PromotePlaceholderToInstance(vertToPromote);
	}

	depth++;
	if (depth <= maxFillDepth) 
	{
		//Search promoted verts for PH neighbors
		for (int i = 0; i < connectedPHs.Num(); i++)
		{
			AVertex* promotedVert = connectedPHs[i];
			for (int j = 0; j < 6; j++)
			{
				FString neighborCoord = GetCoordInDirection(promotedVert, static_cast<EHexagonDirection>(j));
				if (AVertex* neighbor = GetVertex(neighborCoord))
				{
					if (neighbor->type == EVertexType::PlaceHolder)
					{
						FillConnections(neighbor, depth);
					}
				}
			}
		}
	}
}

void AHexGraph::OnZoom(const FInputActionValue& value)
{
	float zoomValue = value.Get<float>();

	ZoomPercent += zoomValue;
	//ZoomPercent = FMath::Clamp<float>(ZoomPercent, -250.0f, 500.0f);

	//Blend our camera's FOV and our SpringArm's length based on ZoomFactor
	//CameraComp->FieldOfView = FMath::Lerp<float>(90.0f, 60.0f, ZoomPercent/100.f);
	SpringArmComp->TargetArmLength = FMath::Lerp<float>(400.0f, 300.0f, ZoomPercent/100.f);
}

void AHexGraph::OnRotate()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	float mouseX, mouseY;
	PlayerController->GetInputMouseDelta(mouseX, mouseY);

	FRotator NewRotation = StaticMeshComp->GetComponentRotation();
	NewRotation.Yaw += mouseX;
	NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + mouseY, 0.f, 85.f);
	//SpringArmComp->SetRelativeRotation(NewRotation);
	Rotation = NewRotation;
}

void AHexGraph::OnMoveForward(const FInputActionValue& value)
{
	FVector newLocation = StaticMeshComp->GetComponentLocation();
	FVector CameraForward = CameraComp->GetComponentTransform().GetUnitAxis(EAxis::X);
	newLocation += CameraForward * value.Get<float>() * PanSpeed;
	if (newLocation.Z < 0) {
		newLocation.Z = 0;
	}
	StaticMeshComp->SetRelativeLocation(newLocation);
}

void AHexGraph::OnMoveBack(const FInputActionValue& value)
{
	FVector newLocation = StaticMeshComp->GetComponentLocation();
	FVector CameraForward = CameraComp->GetComponentTransform().GetUnitAxis(EAxis::X);
	newLocation -= CameraForward * value.Get<float>() * PanSpeed;
	if (newLocation.Z < 0) {
		newLocation.Z = 0;
	}
	StaticMeshComp->SetRelativeLocation(newLocation);
}

void AHexGraph::OnMoveLeft(const FInputActionValue& value)
{
	FVector newLocation = StaticMeshComp->GetComponentLocation();
	FVector CameraRight = CameraComp->GetComponentTransform().GetUnitAxis(EAxis::Y);
	newLocation -= CameraRight * value.Get<float>() * PanSpeed;
	if (newLocation.Z < 0) {
		newLocation.Z = 0;
	}
	StaticMeshComp->SetRelativeLocation(newLocation);
}

void AHexGraph::OnMoveRight(const FInputActionValue& value)
{
	FVector newLocation = StaticMeshComp->GetComponentLocation();
	FVector CameraRight = CameraComp->GetComponentTransform().GetUnitAxis(EAxis::Y);
	newLocation += CameraRight * value.Get<float>() * PanSpeed;
	if (newLocation.Z < 0) {
		newLocation.Z = 0;
	}
	StaticMeshComp->SetRelativeLocation(newLocation);
}

void AHexGraph::DrawTempPiece()
{
	UE_LOG(LogHexGraph, Log, TEXT("MAKE THIS ACTUALLY DRAW"));
}

void AHexGraph::PreviewLineDraw() 
{
	if (lineDrawActivated && !hoverTarget && previousVertexSelection) {
		//Get mouse position projected onto this graphs XY plane
		FVector mousePositionOnGraph = ProjectMousePositionToActorXY(this);

		UE_LOG(LogHexGraph, Log, TEXT("Mouse Project: %f  -  %f"), mousePositionOnGraph.X, mousePositionOnGraph.Y);

		//Determine position of most recent preview or last selected vertex
		AVertex* selectedActor;
		if (previewVertices.Num() > 0) {
			selectedActor = previewVertices.Last();
		}
		else {
			selectedActor = previousVertexSelection;
		}

		UE_LOG(LogHexGraph, Log, TEXT("Selected Actor -> %s"), *selectedActor->GetName());

		FVector selectPosition = selectedActor->GetActorLocation();

		UE_LOG(LogHexGraph, Log, TEXT("Select: %f  -  %f"), selectPosition.X, selectPosition.Y);
		
		//Get the closest hex direction toward the mouse from the selectPosition
		EHexagonDirection hexDirectionToCursor = DetermineHexagonSide(mousePositionOnGraph - selectPosition);
		FString directionName;
		UEnum::GetValueAsString(hexDirectionToCursor, directionName);
		UE_LOG(LogHexGraph, Log, TEXT("Direction: %s"), *directionName);

		FString previewCoord = GetCoordInDirection(selectedActor, hexDirectionToCursor);

		AVertex* newPreview = AddVertexInDirection(previewVertexClass, selectedActor, hexDirectionToCursor, true);
		previewVertices.Add(newPreview);
		lastPreview = newPreview;
		
	}	

	if (useMouseFollower) {
		mouseFollower->SetActorLocation(ProjectMousePositionToActorXY(this));
	}
}

EHexagonDirection AHexGraph::DetermineHexagonSide(const FVector& directionVector)
{
	double Angle = FMath::RadiansToDegrees(FMath::Acos(directionVector.CosineAngle2D(FVector::XAxisVector)));
	Angle = Angle * (directionVector.Y / FMath::Abs(directionVector.Y));
	if (Angle < 0) {
		Angle += 360.f;
	}

	int Sector = FMath::RoundToInt(Angle / 60.f) % 6;
	

	UE_LOG(LogHexGraph, Log, TEXT("\n\nANGLE: %f\nSector: %d\n\n"), Angle, Sector);


	return static_cast<EHexagonDirection>(Sector);
}

FVector AHexGraph::GetUnitDirection(FVector from, FVector to) 
{
	return (to - from).GetSafeNormal();
}

void AHexGraph::CommitTempToGraph()
{
	AVertex* newVertex = nullptr;
	for (auto& element : tempVertices) {
		FString tempCoord = element.Key;
		AVertex* tempVertex = element.Value;

		AVertex* vertexBeneath = GetVertex(tempCoord);
		if(vertexBeneath && vertexBeneath->type == EVertexType::PlaceHolder)
		{
			newVertex = PromotePlaceholderToInstance(Cast<APlaceHolderVertex>(vertexBeneath));
		}
		else {
			newVertex = AddVertexByCoord(defaultVertexClass, tempCoord, tempVertex->GetActorTransform());
		}
	}
	previousVertexSelection = newVertex;
	ClearLineDraw();
}

FVector AHexGraph::ProjectMousePositionToActorXY(AActor* actor)
{
	if (!actor) {
		UE_LOG(LogHexGraph, Error, TEXT("Target actor is null"));
		return FVector::ZeroVector;
	}

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	/*float screenX, screenY;
	PlayerController->GetMousePosition(screenX, screenY);*/

	int32 ViewportX, ViewportY;
	PlayerController->GetViewportSize(ViewportX, ViewportY);

	FVector CameraLocation = CameraComp->GetComponentLocation();
	FRotator CameraRotation = CameraComp->GetComponentRotation();

	FVector WorldDirection;
	FVector WorldLocation;
	PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

	FVector PlaneNormal = actor->GetActorUpVector();
	FVector PlaneLocation = actor->GetActorLocation();

	float Distance = FVector::DotProduct(PlaneNormal, (PlaneLocation - CameraLocation)) / FVector::DotProduct(PlaneNormal, WorldDirection);
	
	FVector Intersection = CameraLocation + Distance * WorldDirection;
	FVector DirectionFromCameraToInterect = GetUnitDirection(CameraLocation, Intersection);

	double stepsToXYPlane = Intersection.Z / DirectionFromCameraToInterect.Z;
	Intersection -= DirectionFromCameraToInterect * stepsToXYPlane;


	return Intersection;
}

void AHexGraph::OnVertexClicked(AActor* clickedActor, FKey clickedButton) 
{
	AVertex* clickedVertex = Cast<AVertex>(clickedActor);
	FName buttonName = clickedButton.GetFName();

	UE_LOG(LogHexGraph, Log, TEXT("Vertex %s was clicked: %s"), *clickedVertex->Coord(), *buttonName.ToString());

	if (buttonName == TEXT("LeftMouseButton")) {
		HandleVertexLeftClick(clickedVertex);
	}
	else if (buttonName == TEXT("RightMouseButton")) {
		HandleVertexRightClick(clickedVertex);
	}
}

void AHexGraph::OnVertexHoverBegin(AActor* hoveredVertex)
{
	hoverTarget = Cast<AVertex>(hoveredVertex);

	/*APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	
	if (PlayerController->IsInputKeyDown(EKeys::LeftMouseButton)) {
		HandleVertexLeftClick(Cast<AVertex>(hoveredVertex));
	}*/

}

void AHexGraph::OnVertexHoverEnd(AActor* hoveredVertex)
{
	hoverTarget = nullptr;
}

void AHexGraph::HandleVertexLeftClick(AVertex* clickedVertex)
{
	if (!clickedVertex)
	{
		return;
	}

	UE_LOG(LogHexGraph, Log, TEXT("Adjacencies for clicked Vertex:\n %s"), *(*adjacencyMatrix.Find(clickedVertex->Coord()))->adjacencyString());

	APlaceHolderVertex* placeHolderVertex = Cast<APlaceHolderVertex>(clickedVertex);

	if (placeHolderVertex) {
		// Use VertexManager if available
		if (VertexManager)
		{
			VertexManager->PromoteToGraphVertex(placeHolderVertex);
		}
		else
		{
			// Fallback to legacy method
			PromotePlaceholderToInstance(placeHolderVertex);
		}
	}
	else {
		UE_LOG(LogHexGraph, Log, TEXT("Instance Vertex Left Clicked"))
	}
}

FString AHexGraph::intsToCoordString(int row, int col)
{
	return  FString::Printf(TEXT("%d:%d"), row, col);
}

void AHexGraph::coordStringToInts(const FString coord, int& row, int& col)
{
	// Initialize output values to invalid state
	row = 0;
	col = 0;
	
	// Validate coordinate string format
	if (!UHexGraphValidation::IsValidCoordinateString(coord))
	{
		UE_LOG(LogHexGraph, Error, TEXT("coordStringToInts: Invalid coordinate string format '%s'"), *coord);
		return;
	}
	
	// Parse the coordinate string
	TArray<FString> coordParts;
	coord.ParseIntoArray(coordParts, TEXT(":"), true);
	
	// Validate we have exactly 2 parts
	HEXGRAPH_VALIDATE_ARRAY_INDEX(coordParts, 0, );
	HEXGRAPH_VALIDATE_ARRAY_INDEX(coordParts, 1, );
	
	// Convert strings to integers
	row = FCString::Atoi(*coordParts[0]);
	col = FCString::Atoi(*coordParts[1]);
	
	// Validate the resulting coordinates
	if (!UHexGraphValidation::IsValidCoordinate(row, col))
	{
		UE_LOG(LogHexGraph, Error, TEXT("coordStringToInts: Parsed coordinates (%d, %d) are out of valid range"), row, col);
		row = 0;
		col = 0;
	}
}

// Called every frame
void AHexGraph::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Update camera controller if available
	if (CameraController)
	{
		CameraController->UpdateCamera(DeltaTime);
	}
	else
	{
		// Fallback to legacy rotation handling
		StaticMeshComp->SetWorldRotation(Rotation);
	}
	
	// Handle mouse coordinate tracking and drawing system updates
	FString currentCoord = GetCoordFromMousePosition();

	if (currentCoord != hoveredCoord) {
		hoveredCoord = currentCoord;
		
		// Update drawing system if available
		if (DrawingSystem && DrawingSystem->IsDrawingActive())
		{
			FHexCoordinate HexCoord = FHexCoordinate::FromString(currentCoord);
			DrawingSystem->UpdatePreview(HexCoord);
		}
		else if (pieceSelected) {
			// Fallback to legacy drawing
			DrawTempPiece();
		}
	}
}

void AHexGraph::InitializeHUD()
{
	if (IsLocallyControlled() && GraphEditorHUDClass) {
		GraphEditorHUD = CreateWidget<UGraphEditorHUD>(GetWorld(), GraphEditorHUDClass);
		GraphEditorHUD->AddToPlayerScreen();
	}
}

void AHexGraph::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Use InputHandler if available, otherwise fall back to direct binding
		if (InputHandler)
		{
			InputHandler->SetupInputBindings(EnhancedInputComponent);
		}
		else
		{
			// Fallback to original direct bindings
			//Select
			EnhancedInputComponent->BindAction(ia_Select, ETriggerEvent::Triggered, this, &AHexGraph::OnSelect);
			
			//LineDraw
			EnhancedInputComponent->BindAction(ia_StartLineDraw, ETriggerEvent::Started, this, &AHexGraph::OnStartLineDraw);
			EnhancedInputComponent->BindAction(ia_StartLineDraw, ETriggerEvent::Triggered, this, &AHexGraph::PreviewLineDraw);
			EnhancedInputComponent->BindAction(ia_StartLineDraw, ETriggerEvent::Completed, this, &AHexGraph::OnStopLineDraw);
			EnhancedInputComponent->BindAction(ia_StartLineDraw, ETriggerEvent::Canceled, this, &AHexGraph::OnStopLineDraw);

			//Delete
			EnhancedInputComponent->BindAction(ia_Delete, ETriggerEvent::Triggered, this, &AHexGraph::OnDelete);

			//Camera Controls
			EnhancedInputComponent->BindAction(ia_Zoom, ETriggerEvent::Triggered, this, &AHexGraph::OnZoom);
			EnhancedInputComponent->BindAction(ia_Rotate, ETriggerEvent::Triggered, this, &AHexGraph::OnRotate);
			EnhancedInputComponent->BindAction(ia_MoveForward, ETriggerEvent::Triggered, this, &AHexGraph::OnMoveForward);
			EnhancedInputComponent->BindAction(ia_MoveBack, ETriggerEvent::Triggered, this, &AHexGraph::OnMoveBack);
			EnhancedInputComponent->BindAction(ia_MoveLeft, ETriggerEvent::Triggered, this, &AHexGraph::OnMoveLeft);
			EnhancedInputComponent->BindAction(ia_MoveRight, ETriggerEvent::Triggered, this, &AHexGraph::OnMoveRight);

			//Fill
			EnhancedInputComponent->BindAction(ia_Fill, ETriggerEvent::Started, this, &AHexGraph::OnFill);
		}
	}
}

FVector2D AHexGraph::GetUnitVectorInHexDirection(int direction)
{
	float Angle = direction * 60.0f * PI / 180.0f;
	FVector2D UnitVector(FMath::Cos(Angle), FMath::Sin(Angle));
	return UnitVector;
}

void AHexGraph::InitializeManagers()
{
	// Create manager instances as default subobjects
	VertexManager = CreateDefaultSubobject<UHexVertexManager>(TEXT("VertexManager"));
	InputHandler = CreateDefaultSubobject<UHexInputHandler>(TEXT("InputHandler"));
	CameraController = CreateDefaultSubobject<UHexCameraController>(TEXT("CameraController"));
	DrawingSystem = CreateDefaultSubobject<UHexDrawingSystem>(TEXT("DrawingSystem"));

	UE_LOG(LogHexGraph, Log, TEXT("HexGraph: Manager instances created"));
}

void AHexGraph::SetupManagerEventBindings()
{
	if (!VertexManager || !InputHandler || !CameraController || !DrawingSystem)
	{
		UE_LOG(LogHexGraph, Error, TEXT("HexGraph::SetupManagerEventBindings: One or more managers are null"));
		return;
	}

	// Initialize managers with dependencies
	VertexManager->Initialize(this);
	InputHandler->Initialize(this);
	CameraController->Initialize(this, SpringArmComp, CameraComp);
	DrawingSystem->Initialize(this, VertexManager);

	// Bind input events to appropriate handlers
	if (InputHandler)
	{
		InputHandler->OnSelectEvent.AddDynamic(this, &AHexGraph::HandleSelectInput);
		InputHandler->OnDeleteEvent.AddDynamic(this, &AHexGraph::HandleDeleteInput);
		InputHandler->OnFillEvent.AddDynamic(this, &AHexGraph::HandleFillInput);
		InputHandler->OnLineDrawStartEvent.AddDynamic(this, &AHexGraph::HandleLineDrawStart);
		InputHandler->OnLineDrawStopEvent.AddDynamic(this, &AHexGraph::HandleLineDrawStop);
		InputHandler->OnZoomEvent.AddDynamic(this, &AHexGraph::HandleZoomInput);
		InputHandler->OnRotateEvent.AddDynamic(this, &AHexGraph::HandleRotateInput);
		InputHandler->OnMoveEvent.AddDynamic(this, &AHexGraph::HandleMoveInput);
	}

	UE_LOG(LogHexGraph, Log, TEXT("HexGraph: Manager event bindings setup complete"));
}

// Manager Event Handler Implementations
void AHexGraph::HandleSelectInput()
{
	// Delegate to original OnSelect logic
	OnSelect();
}

void AHexGraph::HandleDeleteInput()
{
	// Delegate to original OnDelete logic
	OnDelete();
}

void AHexGraph::HandleFillInput()
{
	// Delegate to original OnFill logic
	OnFill();
}

void AHexGraph::HandleLineDrawStart()
{
	// Use DrawingSystem if available
	if (DrawingSystem && hoverTarget)
	{
		FHexCoordinate StartCoord = FHexCoordinate::FromString(hoverTarget->Coord());
		DrawingSystem->StartDrawing(EHexDrawingMode::LineDraw, StartCoord);
	}
	else
	{
		// Fallback to original OnStartLineDraw logic
		OnStartLineDraw();
	}
}

void AHexGraph::HandleLineDrawStop()
{
	// Use DrawingSystem if available
	if (DrawingSystem && DrawingSystem->IsDrawingActive())
	{
		DrawingSystem->StopDrawing(true); // Commit changes
	}
	else
	{
		// Fallback to original OnStopLineDraw logic
		OnStopLineDraw();
	}
}

void AHexGraph::HandleZoomInput(float ZoomDelta)
{
	// Delegate to camera controller
	if (CameraController)
	{
		CameraController->ProcessZoom(ZoomDelta);
	}
	else
	{
		// Fallback to original OnZoom logic
		FInputActionValue Value = FInputActionValue(ZoomDelta);
		OnZoom(Value);
	}
}

void AHexGraph::HandleRotateInput()
{
	// Delegate to camera controller
	if (CameraController)
	{
		CameraController->ProcessRotation();
	}
	else
	{
		// Fallback to original OnRotate logic
		OnRotate();
	}
}

void AHexGraph::HandleMoveInput(FVector2D MovementVector)
{
	// Delegate to camera controller
	if (CameraController)
	{
		CameraController->ProcessMovement(MovementVector);
	}
	else
	{
		// Fallback to original movement logic
		// Convert 2D vector back to individual axis calls
		if (FMath::Abs(MovementVector.X) > 0.1f)
		{
			FInputActionValue Value = FInputActionValue(static_cast<float>(MovementVector.X));
			if (MovementVector.X > 0)
			{
				OnMoveForward(Value);
			}
			else
			{
				OnMoveBack(Value);
			}
		}
		
		if (FMath::Abs(MovementVector.Y) > 0.1f)
		{
			FInputActionValue Value = FInputActionValue(static_cast<float>(MovementVector.Y));
			if (MovementVector.Y > 0)
			{
				OnMoveRight(Value);
			}
			else
			{
				OnMoveLeft(Value);
			}
		}
	}
}

