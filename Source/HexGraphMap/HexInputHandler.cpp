// Copyright Epic Games, Inc. All Rights Reserved.

#include "HexInputHandler.h"
#include "HexGraph.h"
#include "HexGraphMap.h"
#include "EnhancedInputComponent.h"

UHexInputHandler::UHexInputHandler()
{
	OwningHexGraph = nullptr;
	CurrentInputMode = EHexInputMode::Normal;
	bInputEnabled = true;
	bLineDrawActive = false;
}

void UHexInputHandler::Initialize(AHexGraph* InOwningHexGraph)
{
	OwningHexGraph = InOwningHexGraph;
	
	if (!OwningHexGraph.Get())
	{
		UE_LOG(LogHexGraph, Error, TEXT("HexInputHandler::Initialize: OwningHexGraph is null"));
		return;
	}

	// Load input actions from the HexGraph
	LoadInputActionsFromHexGraph();

	UE_LOG(LogHexGraph, Log, TEXT("HexInputHandler: Initialized with HexGraph '%s'"), *OwningHexGraph->GetName());
}

void UHexInputHandler::SetupInputBindings(UEnhancedInputComponent* InputComponent)
{
	if (!InputComponent)
	{
		UE_LOG(LogHexGraph, Error, TEXT("HexInputHandler::SetupInputBindings: InputComponent is null"));
		return;
	}

	if (!OwningHexGraph.Get())
	{
		UE_LOG(LogHexGraph, Error, TEXT("HexInputHandler::SetupInputBindings: OwningHexGraph is null"));
		return;
	}

	// Bind select action
	if (SelectAction)
	{
		InputComponent->BindAction(SelectAction, ETriggerEvent::Triggered, this, &UHexInputHandler::HandleSelectInput);
	}

	// Bind delete action
	if (DeleteAction)
	{
		InputComponent->BindAction(DeleteAction, ETriggerEvent::Triggered, this, &UHexInputHandler::HandleDeleteInput);
	}

	// Bind fill action
	if (FillAction)
	{
		InputComponent->BindAction(FillAction, ETriggerEvent::Started, this, &UHexInputHandler::HandleFillInput);
	}

	// Bind line draw actions
	if (LineDrawAction)
	{
		InputComponent->BindAction(LineDrawAction, ETriggerEvent::Started, this, &UHexInputHandler::HandleLineDrawStart);
		InputComponent->BindAction(LineDrawAction, ETriggerEvent::Triggered, this, &UHexInputHandler::HandleLineDrawOngoing);
		InputComponent->BindAction(LineDrawAction, ETriggerEvent::Completed, this, &UHexInputHandler::HandleLineDrawStop);
		InputComponent->BindAction(LineDrawAction, ETriggerEvent::Canceled, this, &UHexInputHandler::HandleLineDrawCancel);
	}

	// Bind camera controls
	if (ZoomAction)
	{
		InputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &UHexInputHandler::HandleZoomInput);
	}

	if (RotateAction)
	{
		InputComponent->BindAction(RotateAction, ETriggerEvent::Triggered, this, &UHexInputHandler::HandleRotateInput);
	}

	if (MoveForwardAction)
	{
		InputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &UHexInputHandler::HandleMoveForwardInput);
	}

	if (MoveBackAction)
	{
		InputComponent->BindAction(MoveBackAction, ETriggerEvent::Triggered, this, &UHexInputHandler::HandleMoveBackInput);
	}

	if (MoveLeftAction)
	{
		InputComponent->BindAction(MoveLeftAction, ETriggerEvent::Triggered, this, &UHexInputHandler::HandleMoveLeftInput);
	}

	if (MoveRightAction)
	{
		InputComponent->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &UHexInputHandler::HandleMoveRightInput);
	}

	UE_LOG(LogHexGraph, Log, TEXT("HexInputHandler: Input bindings setup complete"));
}

void UHexInputHandler::SetInputMode(EHexInputMode NewMode)
{
	if (CurrentInputMode != NewMode)
	{
		EHexInputMode PreviousMode = CurrentInputMode;
		CurrentInputMode = NewMode;

		// Handle mode transitions
		if (PreviousMode == EHexInputMode::LineDraw && NewMode != EHexInputMode::LineDraw)
		{
			// Exit line draw mode
			bLineDrawActive = false;
			OnLineDrawStopEvent.Broadcast();
		}

		UE_LOG(LogHexGraph, Log, TEXT("HexInputHandler: Input mode changed from %d to %d"), 
			   static_cast<int32>(PreviousMode), static_cast<int32>(NewMode));
	}
}

void UHexInputHandler::HandleSelectInput()
{
	if (!ShouldProcessInput())
	{
		return;
	}

	UE_LOG(LogHexGraph, VeryVerbose, TEXT("HexInputHandler: Select input received"));
	OnSelectEvent.Broadcast();
}

void UHexInputHandler::HandleDeleteInput()
{
	if (!ShouldProcessInput())
	{
		return;
	}

	UE_LOG(LogHexGraph, VeryVerbose, TEXT("HexInputHandler: Delete input received"));
	OnDeleteEvent.Broadcast();
}

void UHexInputHandler::HandleFillInput()
{
	if (!ShouldProcessInput())
	{
		return;
	}

	UE_LOG(LogHexGraph, VeryVerbose, TEXT("HexInputHandler: Fill input received"));
	OnFillEvent.Broadcast();
}

void UHexInputHandler::HandleLineDrawStart()
{
	if (!ShouldProcessInput())
	{
		return;
	}

	UE_LOG(LogHexGraph, Log, TEXT("HexInputHandler: Line draw started"));
	bLineDrawActive = true;
	SetInputMode(EHexInputMode::LineDraw);
	OnLineDrawStartEvent.Broadcast();
}

void UHexInputHandler::HandleLineDrawOngoing()
{
	if (!ShouldProcessInput() || !bLineDrawActive)
	{
		return;
	}

	// Line draw preview updates are handled by the drawing system
	// This is called continuously while the line draw input is held
}

void UHexInputHandler::HandleLineDrawStop()
{
	if (!bLineDrawActive)
	{
		return;
	}

	UE_LOG(LogHexGraph, Log, TEXT("HexInputHandler: Line draw stopped"));
	bLineDrawActive = false;
	SetInputMode(EHexInputMode::Normal);
	OnLineDrawStopEvent.Broadcast();
}

void UHexInputHandler::HandleLineDrawCancel()
{
	if (!bLineDrawActive)
	{
		return;
	}

	UE_LOG(LogHexGraph, Log, TEXT("HexInputHandler: Line draw cancelled"));
	bLineDrawActive = false;
	SetInputMode(EHexInputMode::Normal);
	OnLineDrawStopEvent.Broadcast();
}

void UHexInputHandler::HandleZoomInput(const FInputActionValue& Value)
{
	if (!ShouldProcessInput())
	{
		return;
	}

	float ZoomDelta = Value.Get<float>();
	UE_LOG(LogHexGraph, VeryVerbose, TEXT("HexInputHandler: Zoom input received: %f"), ZoomDelta);
	OnZoomEvent.Broadcast(ZoomDelta);
}

void UHexInputHandler::HandleRotateInput()
{
	if (!ShouldProcessInput())
	{
		return;
	}

	UE_LOG(LogHexGraph, VeryVerbose, TEXT("HexInputHandler: Rotate input received"));
	OnRotateEvent.Broadcast();
}

void UHexInputHandler::HandleMoveForwardInput(const FInputActionValue& Value)
{
	if (!ShouldProcessInput())
	{
		return;
	}

	FVector2D MovementVector = CalculateMovementVector(Value, TEXT("Forward"));
	OnMoveEvent.Broadcast(MovementVector);
}

void UHexInputHandler::HandleMoveBackInput(const FInputActionValue& Value)
{
	if (!ShouldProcessInput())
	{
		return;
	}

	FVector2D MovementVector = CalculateMovementVector(Value, TEXT("Back"));
	OnMoveEvent.Broadcast(MovementVector);
}

void UHexInputHandler::HandleMoveLeftInput(const FInputActionValue& Value)
{
	if (!ShouldProcessInput())
	{
		return;
	}

	FVector2D MovementVector = CalculateMovementVector(Value, TEXT("Left"));
	OnMoveEvent.Broadcast(MovementVector);
}

void UHexInputHandler::HandleMoveRightInput(const FInputActionValue& Value)
{
	if (!ShouldProcessInput())
	{
		return;
	}

	FVector2D MovementVector = CalculateMovementVector(Value, TEXT("Right"));
	OnMoveEvent.Broadcast(MovementVector);
}

bool UHexInputHandler::ShouldProcessInput() const
{
	// Check if input is enabled
	if (!bInputEnabled)
	{
		return false;
	}

	// Check if owning HexGraph is valid
	if (!OwningHexGraph.Get())
	{
		return false;
	}

	// Add any additional input validation logic here
	// For example, checking if the HexGraph is in a valid state for input

	return true;
}

void UHexInputHandler::LoadInputActionsFromHexGraph()
{
	if (!OwningHexGraph.Get())
	{
		UE_LOG(LogHexGraph, Error, TEXT("HexInputHandler::LoadInputActionsFromHexGraph: OwningHexGraph is null"));
		return;
	}

	// Load input actions from the HexGraph
	SelectAction = OwningHexGraph->ia_Select;
	DeleteAction = OwningHexGraph->ia_Delete;
	FillAction = OwningHexGraph->ia_Fill;
	LineDrawAction = OwningHexGraph->ia_StartLineDraw;
	ZoomAction = OwningHexGraph->ia_Zoom;
	RotateAction = OwningHexGraph->ia_Rotate;
	MoveForwardAction = OwningHexGraph->ia_MoveForward;
	MoveBackAction = OwningHexGraph->ia_MoveBack;
	MoveLeftAction = OwningHexGraph->ia_MoveLeft;
	MoveRightAction = OwningHexGraph->ia_MoveRight;

	// Validate that critical actions are loaded
	if (!SelectAction)
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexInputHandler: SelectAction is null"));
	}

	if (!LineDrawAction)
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexInputHandler: LineDrawAction is null"));
	}

	UE_LOG(LogHexGraph, Log, TEXT("HexInputHandler: Input actions loaded from HexGraph"));
}

FVector2D UHexInputHandler::CalculateMovementVector(const FInputActionValue& Value, const FString& Direction) const
{
	float InputValue = Value.Get<float>();
	
	if (Direction == TEXT("Forward"))
	{
		return FVector2D(InputValue, 0.0f);
	}
	else if (Direction == TEXT("Back"))
	{
		return FVector2D(-InputValue, 0.0f);
	}
	else if (Direction == TEXT("Left"))
	{
		return FVector2D(0.0f, -InputValue);
	}
	else if (Direction == TEXT("Right"))
	{
		return FVector2D(0.0f, InputValue);
	}

	return FVector2D::ZeroVector;
}