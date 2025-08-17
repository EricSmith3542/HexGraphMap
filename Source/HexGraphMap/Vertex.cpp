#include "Vertex.h"

// Constructor
AVertex::AVertex()
{
    // No implementation needed for the constructor of an abstract class
}

// Called when the game starts or when spawned
void AVertex::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void AVertex::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

FString AVertex::Coord()
{
    return  FString::Printf(TEXT("%d:%d"), row, col);
}

