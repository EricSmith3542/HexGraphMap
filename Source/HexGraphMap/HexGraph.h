// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/SpringArmComponent.h"
#include "GraphEditorHUD.h"
#include "Camera/CameraComponent.h"
#include "GraphVertex.h"
#include "PlaceHolderVertex.h"
#include "AdjacencyMap.h"
#include "HexGraphSettings.h"
#include "HexVertexManager.h"
#include "HexInputHandler.h"
#include "HexCameraController.h"
#include "HexDrawingSystem.h"
#include "Containers/Map.h"
#include "Templates/SharedPointer.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Vector2D.h"
#include "InputAction.h"
#include <InputMappingContext.h>
#include "HexGraph.generated.h"

UCLASS()
class HEXGRAPHMAP_API AHexGraph : public APawn
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHexGraph();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex Graph")
	TSubclassOf<AGraphVertex> defaultVertexClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex Graph")
	TSubclassOf<APlaceHolderVertex> placeHolderVertexClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex Graph")
	TSubclassOf<APlaceHolderVertex> previewVertexClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex Graph")
	TSet<TSubclassOf<AGraphVertex>> possibleVertices;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FString, AVertex*> vertices;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, UAdjacencyMap*> adjacencyMatrix;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FString, AVertex*> tempVertices;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, UAdjacencyMap*> tempAdjacencyMatrix;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FString, TSubclassOf<AVertex>> selectedPieceVertexClasses;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, UAdjacencyMap*> selectedPieceAdjacencyMatrix;

	// Managed adjacency map tracking for proper cleanup
	UPROPERTY()
	TArray<TObjectPtr<UAdjacencyMap>> ManagedAdjacencyMaps;

	// Manager System Components
	UPROPERTY(BlueprintReadOnly, Category = "Managers")
	TObjectPtr<class UHexVertexManager> VertexManager;

	UPROPERTY(BlueprintReadOnly, Category = "Managers")
	TObjectPtr<class UHexInputHandler> InputHandler;

	UPROPERTY(BlueprintReadOnly, Category = "Managers")
	TObjectPtr<class UHexCameraController> CameraController;

	UPROPERTY(BlueprintReadOnly, Category = "Managers")
	TObjectPtr<class UHexDrawingSystem> DrawingSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex Graph")
	float VertexSpacing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> defaultInputMapping;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ia_Select;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ia_StartLineDraw;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ia_Delete;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ia_Zoom;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ia_Rotate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ia_MoveForward;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ia_MoveBack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ia_MoveLeft;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ia_MoveRight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ia_Fill;

	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	AGraphVertex* AddFirstVertex();

	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	AVertex* AddVertexByRowCol(TSubclassOf<AVertex> vertexClass, int row, int col, FTransform spawnTransform, bool isTemp = false);

	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	AVertex* AddVertexByCoord(TSubclassOf<AVertex> vertexClass, FString coord, FTransform spawnTransform, bool isTemp = false);

	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	AVertex* AddVertexWithAdjacencies(TSubclassOf<AVertex> vertexClass, int row, int col, FTransform spawnTransform, TArray<FString> adjacencies, bool isTemp = false);

	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	AVertex* AddVertexInDirection(TSubclassOf<AVertex> vertexClass, AVertex* rootVertex, EHexagonDirection direction, bool isTemp = false);

	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	AVertex* GetVertex(FString coord);

	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	void RemoveVertexAtCoord(FString coord);

	// FHexCoordinate overloads for improved type safety
	AVertex* GetVertex(const FHexCoordinate& Coordinate);
	void RemoveVertexAtCoord(const FHexCoordinate& Coordinate);
	AVertex* AddVertexByCoord(TSubclassOf<AVertex> vertexClass, const FHexCoordinate& Coordinate, FTransform spawnTransform, bool isTemp = false);

	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	UAdjacencyMap* GetAdjacenciesForVertex(AVertex* vertex);

	// Managed adjacency map system
	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	UAdjacencyMap* CreateManagedAdjacencyMap();

	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	void CleanupManagedAdjacencyMaps();

	// Vertex reference management
	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	void ValidateAllVertexReferences();

	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	int32 CleanupInvalidVertexReferences();

	// Settings helpers
	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	void RefreshSettingsValues();
	
	// Console command for manual settings refresh
	UFUNCTION(Exec, BlueprintCallable, Category = "HexGraph")
	void RefreshSettings();
	
	UFUNCTION(BLueprintCallable, Category = "HexGraph")
	void InitializeVertex(AVertex* vertex, int row, int col, bool isTemp = false, UAdjacencyMap* adjacencyMap = nullptr);

	UFUNCTION(BLueprintCallable, Category = "HexGraph")
	void OnVertexClicked(AActor* clickedVertex, FKey clickedButton);

	UFUNCTION(BLueprintCallable, Category = "HexGraph")
	void OnVertexHoverBegin(AActor* hoveredVertex);

	UFUNCTION(BLueprintCallable, Category = "HexGraph")
	void OnVertexHoverEnd(AActor* hoveredVertex);

	UFUNCTION(BLueprintCallable, Category = "HexGraph")
	void HandleVertexLeftClick(AVertex* clickedVertex);

	UFUNCTION(BLueprintCallable, Category = "HexGraph")
	void HandleVertexRightClick(AVertex* clickedVertex);

	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	AVertex* PromotePlaceholderToInstance(APlaceHolderVertex* placeHolderVertex);

	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	APlaceHolderVertex* DemoteInstanceToPlaceHolder(AGraphVertex* graphVertex);

	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	FString GetCoordInDirection(AVertex* vert, EHexagonDirection direction);

	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	FString GetCoordFromMousePosition();

	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	void ListConnectedVertices(TArray<AVertex*>& connected, AVertex* startingVertex, const TSet<EVertexType>& includedTypes);
	
	UFUNCTION(BlueprintCallable, Category = "HexGraph")
	void SelectGraphPiece(TMap<FString, TSubclassOf<AVertex>> pieceVertices, TMap<FString, FString> stringifiedAdjacencyMap, FString pieceName);

	FActorSpawnParameters BuildVertexSpawnParams();

	FVector2D GetUnitVectorInHexDirection(int direction);

	FString intsToCoordString(int row, int col);

	EHexagonDirection DetermineHexagonSide(const FVector& directionVector);

	void coordStringToInts(const FString coord, int& row, int& col);

	static int verticesCreated;

	float MeshLength;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex Graph Debug")
	bool useMouseFollower;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex Graph Debug")
	TSubclassOf<AActor> mouseFollowerClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex Graph")
	int maxFillDepth;

	//UI Components
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex Graph")
	TSubclassOf<class UGraphEditorHUD> GraphEditorHUDClass;

	UPROPERTY()
	UGraphEditorHUD* GraphEditorHUD;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent);

	void OnSelect();
	void OnDelete();

	//Line Draw functions
	void OnStartLineDraw();
	void OnStopLineDraw();
	void PreviewLineDraw();
	void ClearLineDraw();
	FVector ProjectMousePositionToActorXY(AActor* actor);
	FVector GetUnitDirection(FVector from, FVector to);
	TArray<AVertex*> previewVertices;
	AVertex* lastPreview;

	void OnFill();
	void FillConnections(AVertex* vert, int depth = 0);

	void CommitTempToGraph();

	AVertex* hoverTarget;
	FString hoveredCoord;

	AVertex* previousVertexSelection;

	AActor* mouseFollower;

	bool lineDrawActivated;
	bool deleting;

	bool pieceSelected;
	void DrawTempPiece();

	//Camera Components
	UPROPERTY(EditAnywhere)
	USpringArmComponent* SpringArmComp;

	UPROPERTY(EditAnywhere)
	UCameraComponent* CameraComp;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* StaticMeshComp;

	//Camera Input Functions
	void OnZoom(const FInputActionValue& value);
	void OnRotate();
	void OnMoveForward(const FInputActionValue& value);
	void OnMoveBack(const FInputActionValue& value);
	void OnMoveLeft(const FInputActionValue& value);
	void OnMoveRight(const FInputActionValue& value);
	float ZoomPercent = 100.f;
	float PanSpeed = 15.f;
	FRotator Rotation = FRotator();

	void InitializeHUD();

	// Manager System Functions
	void InitializeManagers();
	void SetupManagerEventBindings();

	// Manager Event Handlers
	UFUNCTION()
	void HandleSelectInput();
	
	UFUNCTION()
	void HandleDeleteInput();
	
	UFUNCTION()
	void HandleFillInput();
	
	UFUNCTION()
	void HandleLineDrawStart();
	
	UFUNCTION()
	void HandleLineDrawStop();
	
	UFUNCTION()
	void HandleZoomInput(float ZoomDelta);
	
	UFUNCTION()
	void HandleRotateInput();
	
	UFUNCTION()
	void HandleMoveInput(FVector2D MovementVector);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
