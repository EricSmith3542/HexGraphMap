// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HexagonDirection.h"
#include "Vertex.h"
#include "GraphVertex.generated.h"

/**
 * 
 */
UCLASS()
class HEXGRAPHMAP_API AGraphVertex : public AVertex
{
	GENERATED_BODY()

public:
	AGraphVertex();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
