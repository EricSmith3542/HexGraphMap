// Fill out your copyright notice in the Description page of Project Settings.


#include "HexGraph.h"
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

	MeshLength = -1;
	VertexSpacing = 0.5f;
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

	maxFillDepth = 5;
}

AGraphVertex* AHexGraph::AddFirstVertex()
{

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

		GetAdjacenciesForVertex(newPlaceHolderVert)->setAllAdjacencies(placeHolderAdjacencies);

		firstNeighborDirection = static_cast<EHexagonDirection>((static_cast<int32>(firstNeighborDirection) + 1) % 6);
		secondNeighborDirection = static_cast<EHexagonDirection>((static_cast<int32>(secondNeighborDirection) + 1) % 6);
		thirdNeighborDirection = static_cast<EHexagonDirection>((static_cast<int32>(thirdNeighborDirection) + 1) % 6);
	}

	return firstVertex;
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

void AHexGraph::EndPlay(const EEndPlayReason::Type)
{
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
	return vertices.Contains(coord) ? *vertices.Find(coord):nullptr;
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
							TArray<FString> phNeighbors = GetAdjacenciesForVertex(neighborVert)->adjacentVertexCoords;
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
						TArray<FString> neighborAdjacencies = GetAdjacenciesForVertex(neighborVert)->adjacentVertexCoords;
						for (FString& adjacencyCoord : neighborAdjacencies) {
							if (adjacencyCoord == coord) {
								adjacencyCoord = "";
							}
						}
						GetAdjacenciesForVertex(neighborVert)->setAllAdjacencies(neighborAdjacencies);
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
	return *adjacencyMatrix.Find(vertex->Coord());
}

//Adds a vertex to the map and sets its adjacencies to the given list; adds adjacencies for all neighbors if forceBiDirectionalAdjacency is set to true
AVertex* AHexGraph::AddVertexWithAdjacencies(TSubclassOf<AVertex> vertexClass, int row, int col, FTransform spawnTransform, TArray<FString> adjacencies, bool isTemp)
{
	AVertex* newVertex = AddVertexByRowCol(vertexClass, row, col, spawnTransform);
	GetAdjacenciesForVertex(newVertex)->setAllAdjacencies(adjacencies);

	return newVertex;
}

void AHexGraph::HandleVertexRightClick(AVertex* clickedVertex)
{
	UE_LOG(LogTemp, Log, TEXT("Adjacencies for clicked Vertex:\n %s"), *(*adjacencyMatrix.Find(clickedVertex->Coord()))->adjacencyString());
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
					GetAdjacenciesForVertex(GetVertex(coord))->setDirectionsAdjacency((j + 3)%6, newPlaceHolder->Coord());
				}
			}
			GetAdjacenciesForVertex(newPlaceHolder)->setAllAdjacencies(placeHolderAdjacencies);
			GetAdjacenciesForVertex(promotedVertex)->setDirectionsAdjacency(i, newPlaceHolder->Coord());

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
		UAdjacencyMap* currentMap = NewObject<UAdjacencyMap>(this);
		TArray<FString> currentAdj;
		coordAdjacencyPair.Value.ParseIntoArray(currentAdj, TEXT(";"), false);
		currentMap->setAllAdjacencies(currentAdj);

		selectedPieceAdjacencyMatrix.Add(coordAdjacencyPair.Key, currentMap);
	}
}

void AHexGraph::InitializeVertex(AVertex* vertex, int row, int col, bool isTemp, UAdjacencyMap* adjacencyMap)
{
	vertex->row = row;
	vertex->col = col;

	if (isTemp) {
		tempVertices.Add(vertex->Coord(), vertex);
		tempAdjacencyMatrix.Add(vertex->Coord(), (adjacencyMap == nullptr ? NewObject<UAdjacencyMap>(this) : adjacencyMap));
	}
	else {
		vertices.Add(vertex->Coord(), vertex);
		adjacencyMatrix.Add(vertex->Coord(), (adjacencyMap == nullptr ? NewObject<UAdjacencyMap>(this) : adjacencyMap));
	}

	//vertex->OnClicked.AddDynamic(this, &AHexGraph::OnVertexClicked);
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
	UE_LOG(LogTemp, Log, TEXT("\n\nOnDelete START\n\n"));
	if (!deleting && hoverTarget && hoverTarget->type == EVertexType::Graph) {
		deleting = true;
		RemoveVertexAtCoord(hoverTarget->Coord());
		hoverTarget = nullptr;
		deleting = false;
	}
}

void AHexGraph::OnStartLineDraw()
{
	UE_LOG(LogTemp, Log, TEXT("\n\nLINE DRAW START\n\n"));
	lineDrawActivated = true;
}

void AHexGraph::OnStopLineDraw()
{
	UE_LOG(LogTemp, Log, TEXT("\n\nLINE DRAW END\n\n"));
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

	UE_LOG(LogTemp, Log, TEXT("Coord: %s"), *mouseCoord);
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
	UE_LOG(LogTemp, Log, TEXT("MAKE THIS ACTUALLY DRAW"));
}

void AHexGraph::PreviewLineDraw() 
{
	if (lineDrawActivated && !hoverTarget && previousVertexSelection) {
		//Get mouse position projected onto this graphs XY plane
		FVector mousePositionOnGraph = ProjectMousePositionToActorXY(this);

		UE_LOG(LogTemp, Log, TEXT("Mouse Project: %f  -  %f"), mousePositionOnGraph.X, mousePositionOnGraph.Y);

		//Determine position of most recent preview or last selected vertex
		AVertex* selectedActor;
		if (previewVertices.Num() > 0) {
			selectedActor = previewVertices.Last();
		}
		else {
			selectedActor = previousVertexSelection;
		}

		UE_LOG(LogTemp, Log, TEXT("Selected Actor -> %s"), *selectedActor->GetActorLabel());

		FVector selectPosition = selectedActor->GetActorLocation();

		UE_LOG(LogTemp, Log, TEXT("Select: %f  -  %f"), selectPosition.X, selectPosition.Y);
		
		//Get the closest hex direction toward the mouse from the selectPosition
		EHexagonDirection hexDirectionToCursor = DetermineHexagonSide(mousePositionOnGraph - selectPosition);
		FString directionName;
		UEnum::GetValueAsString(hexDirectionToCursor, directionName);
		UE_LOG(LogTemp, Log, TEXT("Direction: %s"), *directionName);

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
	

	UE_LOG(LogTemp, Log, TEXT("\n\nANGLE: %f\nSector: %d\n\n"), Angle, Sector);


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
		UE_LOG(LogTemp, Error, TEXT("Target actor is null"));
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

	UE_LOG(LogTemp, Log, TEXT("Vertex %s was clicked: %s"), *clickedVertex->Coord(), *buttonName.ToString());

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
	UE_LOG(LogTemp, Log, TEXT("Adjacencies for clicked Vertex:\n %s"), *(*adjacencyMatrix.Find(clickedVertex->Coord()))->adjacencyString());

	APlaceHolderVertex* placeHolderVertex = Cast<APlaceHolderVertex>(clickedVertex);

	if (placeHolderVertex) {
		PromotePlaceholderToInstance(placeHolderVertex);
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Instance Vertex Left Clicked"))
	}

}

FString AHexGraph::intsToCoordString(int row, int col)
{
	return  FString::Printf(TEXT("%d:%d"), row, col);
}

void AHexGraph::coordStringToInts(const FString coord, int& row, int& col)
{
	TArray<FString> coordParts;
	coord.ParseIntoArray(coordParts, TEXT(":"), true);
	row = FCString::Atoi(*coordParts[0]);
	col = FCString::Atoi(*coordParts[1]);
}

// Called every frame
void AHexGraph::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	StaticMeshComp->SetWorldRotation(Rotation);
	FString currentCoord = GetCoordFromMousePosition();

	if (currentCoord != hoveredCoord) {
		hoveredCoord = currentCoord;
		if (pieceSelected) {
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

FVector2D AHexGraph::GetUnitVectorInHexDirection(int direction)
{
	float Angle = direction * 60.0f * PI / 180.0f;
	FVector2D UnitVector(FMath::Cos(Angle), FMath::Sin(Angle));
	return UnitVector;
}

