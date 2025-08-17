// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Vertex.h"
#include "PlaceHolderVertex.generated.h"

UCLASS()
class HEXGRAPHMAP_API APlaceHolderVertex : public AVertex
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlaceHolderVertex();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
