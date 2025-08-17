// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include <InputMappingContext.h>
#include "HexGraphEditorPlayerController.generated.h"


/**
 * 
 */
UCLASS()
class HEXGRAPHMAP_API AHexGraphEditorPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AHexGraphEditorPlayerController();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HexEditor Input")
	TSoftObjectPtr<UInputMappingContext> inputMapping;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HexEditor Input")
	UInputAction* ia_Select;


protected:
	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;
	
};
