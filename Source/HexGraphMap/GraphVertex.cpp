// Fill out your copyright notice in the Description page of Project Settings.


#include "GraphVertex.h"

// Sets default values
AGraphVertex::AGraphVertex()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Create the static mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent; // Set the root component to the mesh component

    // Set the default mesh (you can set this in the Details panel as well)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Script/Engine.StaticMesh'/ControlRig/Controls/ControlRig_Hexagon_solid.ControlRig_Hexagon_solid'"));
    if (MeshAsset.Succeeded())
    {
        MeshComponent->SetStaticMesh(MeshAsset.Object);
    }

    // Set default values for animation variables
    Amplitude = 50.0f;
    Frequency = 1.0f;

    // Initialize time variable
    TimeSinceStart = 0.0f;

}

// Called when the game starts or when spawned
void AGraphVertex::BeginPlay()
{
	Super::BeginPlay();

    MeshComponent->SetStaticMesh(DefaultMesh);
}

// Called every frame
void AGraphVertex::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    //// Update time
    //TimeSinceStart += DeltaTime;

    //// Calculate vertical offset based on sine function for animation
    //float VerticalOffset = Amplitude * FMath::Sin(Frequency * TimeSinceStart);

    //// Set the new location of the mesh
    //FVector NewLocation = GetActorLocation();
    //NewLocation.Z += VerticalOffset;
    //SetActorLocation(NewLocation);

}
