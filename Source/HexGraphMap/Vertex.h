
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "VertexType.h"
#include "Vertex.generated.h"

UCLASS(Abstract)
class HEXGRAPHMAP_API AVertex : public AActor
{
    GENERATED_BODY()

public:
    AVertex();

    //Default mesh to use for PlaceHolderVertices
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HexGraph")
    UStaticMesh* DefaultMesh;

    // Property for the static mesh component
    UPROPERTY(BlueprintReadOnly, Category = "HexGraph")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HexGraph")
    int row;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HexGraph")
    int col;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HexGraph")
    EVertexType type;

    // Variables for the animation
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Animation")
    float Amplitude;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Animation")
    float Frequency;

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    FString Coord();

protected:

    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    float TimeSinceStart;
};
