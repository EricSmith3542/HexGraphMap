// Copyright Epic Games, Inc. All Rights Reserved.

#include "HexInputHandler.h"
#include "HexGraph.h"
#include "HexGraphMap.h"
#include "HexGraphEventManager.h"
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

	// Use the HexGraph's existing input actions to restore original functionality
	AHexGraph* HexGraph = OwningHexGraph.Get();
	
	// Bind select action
	if (HexGraph && HexGraph->ia_Select)
	{
		InputComponent->BindAction(HexGraph->ia_Select, ETriggerEvent::Triggered, this, &UHexInputHandler::HandleSelectInput);
	}
	else
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexInputHandler: ia_Select is not set in HexGraph"));
	}

	// Bind delete action
	if (HexGraph && HexGraph->ia_Delete)
	{
		InputComponent->BindAction(HexGraph->ia_Delete, ETriggerEvent::Triggered, this, &UHexInputHandler::HandleDeleteInput);
	}
	else
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexInputHandler: ia_Delete is not set in HexGraph"));
	}

	// Bind line draw actions
	if (HexGraph && HexGraph->ia_StartLineDraw)
	{
		InputComponent->BindAction(HexGraph->ia_StartLineDraw, ETriggerEvent::Started, this, &UHexInputHandler::HandleLineDrawStart);
		InputComponent->BindAction(HexGraph->ia_StartLineDraw, ETriggerEvent::Triggered, this, &UHexInputHandler::HandleLineDrawOngoing);
		InputComponent->BindAction(HexGraph->ia_StartLineDraw, ETriggerEvent::Completed, this, &UHexInputHandler::HandleLineDrawStop);
		InputComponent->BindAction(HexGraph->ia_StartLineDraw, ETriggerEvent::Canceled, this, &UHexInputHandler::HandleLineDrawCancel);
	}
	else
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexInputHandler: ia_StartLineDraw is not set in HexGraph"));
	}

	// Bind camera controls
	if (HexGraph && HexGraph->ia_Zoom)
	{
		InputComponent->BindAction(HexGraph->ia_Zoom, ETriggerEvent::Triggered, this, &UHexInputHandler::HandleZoomInput);
	}
	else
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexInputHandler: ia_Zoom is not set in HexGraph"));
	}

	if (HexGraph && HexGraph->ia_Rotate)
	{
		InputComponent->BindAction(HexGraph->ia_Rotate, ETriggerEvent::Triggered, this, &UHexInputHandler::HandleRotateInput);
	}
	else
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexInputHandler: ia_Rotate is not set in HexGraph"));
	}

	if (HexGraph && HexGraph->ia_MoveForward)
	{
		InputComponent->BindAction(HexGraph->ia_MoveForward, ETriggerEvent::Triggered, this, &UHexInputHandler::HandleMoveForwardInput);
	}
	else
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexInputHandler: ia_MoveForward is not set in HexGraph"));
	}

	if (HexGraph && HexGraph->ia_MoveBack)
	{
		InputComponent->BindAction(HexGraph->ia_MoveBack, ETriggerEvent::Triggered, this, &UHexInputHandler::HandleMoveBackInput);
	}
	else
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexInputHandler: ia_MoveBack is not set in HexGraph"));
	}

	if (HexGraph && HexGraph->ia_MoveLeft)
	{
		InputComponent->BindAction(HexGraph->ia_MoveLeft, ETriggerEvent::Triggered, this, &UHexInputHandler::HandleMoveLeftInput);
	}
	else
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexInputHandler: ia_MoveLeft is not set in HexGraph"));
	}

	if (HexGraph && HexGraph->ia_MoveRight)
	{
		InputComponent->BindAction(HexGraph->ia_MoveRight, ETriggerEvent::Triggered, this, &UHexInputHandler::HandleMoveRightInput);
	}
	else
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexInputHandler: ia_MoveRight is not set in HexGraph"));
	}

	// Bind fill action if it exists in HexGraph
	if (HexGraph && HexGraph->ia_Fill)
	{
		InputComponent->BindAction(HexGraph->ia_Fill, ETriggerEvent::Started, this, &UHexInputHandler::HandleFillInput);
	}

	UE_LOG(LogHexGraph, Log, TEXT("HexInputHandler: Input bindings setup complete using HexGraph input actions"));
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

		// Broadcast input mode change event through central event system
		if (OwningHexGraph.Get() && OwningHexGraph->EventManager)
		{
			FString ModeChangeInfo = FString::Printf(TEXT("Input mode changed from %s to %s"), 
				*UEnum::GetValueAsString(PreviousMode), 
				*UEnum::GetValueAsString(NewMode));

			OwningHexGraph->EventManager->BroadcastEvent(
				EHexGraphEventType::InputModeChanged, 
				FHexCoordinate(), 
				nullptr, 
				OwningHexGraph.Get(), 
				ModeChangeInfo
			);
		}

		UE_LOG(LogHexGraph, Log, TEXT("HexInputHandler: Input mode changed from %d to %d"), 
			   static_cast<int32>(PreviousMode), static_cast<int32>(NewMode));
	}
}

void UHexInputHandler::HandleSelectInput()
{
	UE_LOG(LogHexGraph, Warning, TEXT("HexInputHandler: HandleSelectInput called"));
	
	if (!ShouldProcessInput())
	{
		UE_LOG(LogHexGraph, Warning, TEXT("HexInputHandler: ShouldProcessInput returned false"));
		return;
	}

	UE_LOG(LogHexGraph, Warning, TEXT("HexInputHandler: Select input received, broadcasting event"));
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

