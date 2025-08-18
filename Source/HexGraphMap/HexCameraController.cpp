// Copyright Epic Games, Inc. All Rights Reserved.

#include "HexCameraController.h"
#include "HexGraph.h"
#include "HexGraphSettings.h"
#include "HexGraphMap.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Math/UnrealMathUtility.h"

UHexCameraController::UHexCameraController()
{
	OwningHexGraph = nullptr;
	SpringArmComponent = nullptr;
	CameraComponent = nullptr;
	CurrentCameraMode = EHexCameraMode::Free;
	CurrentZoomPercent = 100.0f;
	CurrentPanSpeed = 15.0f;
	CurrentRotation = FRotator::ZeroRotator;
	TargetPosition = FVector::ZeroVector;
	TargetRotation = FRotator::ZeroRotator;
	TargetZoom = 100.0f;
	bUseSmoothTransitions = true;
	TransitionSpeed = 5.0f;
	MinZoomPercent = 10.0f;
	MaxZoomPercent = 500.0f;
	MovementBoundsMin = FVector(-10000.0f, -10000.0f, 0.0f);
	MovementBoundsMax = FVector(10000.0f, 10000.0f, 1000.0f);
	bEnforceMovementBounds = false;
}

void UHexCameraController::Initialize(AHexGraph* InOwningHexGraph, USpringArmComponent* InSpringArm, UCameraComponent* InCamera)
{
	OwningHexGraph = InOwningHexGraph;
	SpringArmComponent = InSpringArm;
	CameraComponent = InCamera;

	if (!OwningHexGraph.Get())
	{
		UE_LOG(LogHexGraph, Error, TEXT("HexCameraController::Initialize: OwningHexGraph is null"));
		return;
	}

	if (!SpringArmComponent.Get())
	{
		UE_LOG(LogHexGraph, Error, TEXT("HexCameraController::Initialize: SpringArmComponent is null"));
		return;
	}

	if (!CameraComponent.Get())
	{
		UE_LOG(LogHexGraph, Error, TEXT("HexCameraController::Initialize: CameraComponent is null"));
		return;
	}

	// Load settings from HexGraph
	LoadCameraSettings();

	// Initialize current values from components
	CurrentRotation = SpringArmComponent->GetRelativeRotation();
	TargetRotation = CurrentRotation;
	TargetPosition = SpringArmComponent->GetComponentLocation();
	TargetZoom = CurrentZoomPercent;

	UE_LOG(LogHexGraph, Log, TEXT("HexCameraController: Initialized with HexGraph '%s'"), *OwningHexGraph->GetName());
}

void UHexCameraController::ProcessZoom(float ZoomDelta)
{
	if (!SpringArmComponent.Get())
	{
		return;
	}

	float NewZoom = CurrentZoomPercent + ZoomDelta;
	NewZoom = FMath::Clamp(NewZoom, MinZoomPercent, MaxZoomPercent);

	if (bUseSmoothTransitions)
	{
		TargetZoom = NewZoom;
	}
	else
	{
		SetZoomLevel(NewZoom, false);
	}

	UE_LOG(LogHexGraph, VeryVerbose, TEXT("HexCameraController: Zoom processed - Delta: %f, New: %f"), ZoomDelta, NewZoom);
}

void UHexCameraController::ProcessRotation()
{
	if (!OwningHexGraph.Get() || !SpringArmComponent.Get())
	{
		return;
	}

	// Get mouse delta from player controller
	APlayerController* PlayerController = OwningHexGraph->GetWorld()->GetFirstPlayerController();
	if (!PlayerController)
	{
		return;
	}

	float MouseX, MouseY;
	PlayerController->GetInputMouseDelta(MouseX, MouseY);

	FRotator NewRotation = CurrentRotation;
	NewRotation.Yaw += MouseX;
	NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + MouseY, -85.0f, 85.0f);

	if (bUseSmoothTransitions)
	{
		TargetRotation = NewRotation;
	}
	else
	{
		SetCameraRotation(NewRotation, false);
	}

	UE_LOG(LogHexGraph, VeryVerbose, TEXT("HexCameraController: Rotation processed - Mouse: (%f, %f), New: %s"), 
		   MouseX, MouseY, *NewRotation.ToString());
}

void UHexCameraController::ProcessMovement(const FVector2D& MovementVector)
{
	if (!CameraComponent.Get() || !SpringArmComponent.Get())
	{
		return;
	}

	// Get camera transform for direction calculation
	FTransform CameraTransform = CameraComponent->GetComponentTransform();
	FVector CameraForward = CameraTransform.GetUnitAxis(EAxis::X);
	FVector CameraRight = CameraTransform.GetUnitAxis(EAxis::Y);

	// Calculate movement based on camera orientation
	FVector MovementDirection = (CameraForward * MovementVector.X) + (CameraRight * MovementVector.Y);
	MovementDirection.Z = 0.0f; // Keep movement in horizontal plane
	MovementDirection = MovementDirection.GetSafeNormal();

	// Apply pan speed
	FVector MovementOffset = MovementDirection * CurrentPanSpeed * MovementVector.Size();
	FVector CurrentPosition = SpringArmComponent->GetComponentLocation();
	FVector NewPosition = CurrentPosition + MovementOffset;

	// Clamp to bounds if enabled
	if (bEnforceMovementBounds)
	{
		NewPosition = ClampToBounds(NewPosition);
	}

	if (bUseSmoothTransitions)
	{
		TargetPosition = NewPosition;
	}
	else
	{
		SetCameraPosition(NewPosition, false);
	}

	UE_LOG(LogHexGraph, VeryVerbose, TEXT("HexCameraController: Movement processed - Vector: %s, New Position: %s"), 
		   *MovementVector.ToString(), *NewPosition.ToString());
}

void UHexCameraController::SetCameraPosition(const FVector& NewPosition, bool bUseSmoothing)
{
	if (!SpringArmComponent.Get())
	{
		return;
	}

	FVector ClampedPosition = bEnforceMovementBounds ? ClampToBounds(NewPosition) : NewPosition;

	if (bUseSmoothing && bUseSmoothTransitions)
	{
		TargetPosition = ClampedPosition;
	}
	else
	{
		ApplyPosition(ClampedPosition);
		OnCameraMoveEvent.Broadcast(ClampedPosition);
	}
}

void UHexCameraController::SetCameraRotation(const FRotator& NewRotation, bool bUseSmoothing)
{
	if (!SpringArmComponent.Get())
	{
		return;
	}

	if (bUseSmoothing && bUseSmoothTransitions)
	{
		TargetRotation = NewRotation;
	}
	else
	{
		ApplyRotation(NewRotation);
		OnCameraRotateEvent.Broadcast(NewRotation);
	}
}

void UHexCameraController::SetZoomLevel(float NewZoomPercent, bool bUseSmoothing)
{
	float ClampedZoom = FMath::Clamp(NewZoomPercent, MinZoomPercent, MaxZoomPercent);

	if (bUseSmoothing && bUseSmoothTransitions)
	{
		TargetZoom = ClampedZoom;
	}
	else
	{
		ApplyZoom(ClampedZoom);
		OnCameraZoomEvent.Broadcast(ClampedZoom);
	}
}

FVector UHexCameraController::GetCameraPosition() const
{
	if (SpringArmComponent.Get())
	{
		return SpringArmComponent->GetComponentLocation();
	}
	return FVector::ZeroVector;
}

FRotator UHexCameraController::GetCameraRotation() const
{
	if (SpringArmComponent.Get())
	{
		return SpringArmComponent->GetRelativeRotation();
	}
	return FRotator::ZeroRotator;
}

void UHexCameraController::SetCameraMode(EHexCameraMode NewMode)
{
	if (CurrentCameraMode != NewMode)
	{
		EHexCameraMode PreviousMode = CurrentCameraMode;
		CurrentCameraMode = NewMode;

		// Handle mode-specific setup
		switch (NewMode)
		{
		case EHexCameraMode::Free:
			bEnforceMovementBounds = false;
			break;
		case EHexCameraMode::Locked:
			bEnforceMovementBounds = true;
			break;
		case EHexCameraMode::Follow:
			// Follow mode implementation would go here
			break;
		}

		UE_LOG(LogHexGraph, Log, TEXT("HexCameraController: Camera mode changed from %d to %d"), 
			   static_cast<int32>(PreviousMode), static_cast<int32>(NewMode));
	}
}

void UHexCameraController::FocusOnPosition(const FVector& FocusTargetPosition, bool bUseSmoothing)
{
	SetCameraPosition(FocusTargetPosition, bUseSmoothing);
}

void UHexCameraController::ResetToDefaults()
{
	if (!SpringArmComponent.Get())
	{
		return;
	}

	// Reset to initial settings
	FVector DefaultPosition = FVector(0.0f, 0.0f, 400.0f);
	FRotator DefaultRotation = FRotator(-90.0f, 0.0f, 0.0f);
	float DefaultZoom = 100.0f;

	if (bUseSmoothTransitions)
	{
		TargetPosition = DefaultPosition;
		TargetRotation = DefaultRotation;
		TargetZoom = DefaultZoom;
	}
	else
	{
		SetCameraPosition(DefaultPosition, false);
		SetCameraRotation(DefaultRotation, false);
		SetZoomLevel(DefaultZoom, false);
	}

	UE_LOG(LogHexGraph, Log, TEXT("HexCameraController: Reset to defaults"));
}

void UHexCameraController::UpdateCamera(float DeltaTime)
{
	if (bUseSmoothTransitions)
	{
		UpdateSmoothTransitions(DeltaTime);
	}
}

void UHexCameraController::ApplyZoom(float ZoomPercent)
{
	if (!SpringArmComponent.Get())
	{
		return;
	}

	CurrentZoomPercent = ZoomPercent;

	// Apply zoom by adjusting spring arm length
	float BaseArmLength = 400.0f;
	float NewArmLength = FMath::Lerp(BaseArmLength * 0.75f, BaseArmLength * 1.25f, ZoomPercent / 100.0f);
	SpringArmComponent->TargetArmLength = NewArmLength;
}

void UHexCameraController::ApplyRotation(const FRotator& Rotation)
{
	if (!SpringArmComponent.Get())
	{
		return;
	}

	CurrentRotation = Rotation;
	SpringArmComponent->SetRelativeRotation(Rotation);
}

void UHexCameraController::ApplyPosition(const FVector& Position)
{
	if (!SpringArmComponent.Get())
	{
		return;
	}

	SpringArmComponent->SetWorldLocation(Position);
}

FVector UHexCameraController::ClampToBounds(const FVector& Position) const
{
	return FVector(
		FMath::Clamp(Position.X, MovementBoundsMin.X, MovementBoundsMax.X),
		FMath::Clamp(Position.Y, MovementBoundsMin.Y, MovementBoundsMax.Y),
		FMath::Clamp(Position.Z, MovementBoundsMin.Z, MovementBoundsMax.Z)
	);
}

void UHexCameraController::LoadCameraSettings()
{
	if (!OwningHexGraph.Get())
	{
		return;
	}

	const UHexGraphSettings* Settings = UHexGraphSettings::GetHexGraphSettings();
	if (Settings)
	{
		CurrentPanSpeed = Settings->DefaultPanSpeed;
		CurrentZoomPercent = Settings->DefaultZoomPercent;
		TargetZoom = CurrentZoomPercent;

		UE_LOG(LogHexGraph, Log, TEXT("HexCameraController: Loaded settings - PanSpeed: %f, ZoomPercent: %f"), 
			   CurrentPanSpeed, CurrentZoomPercent);
	}
}

void UHexCameraController::UpdateSmoothTransitions(float DeltaTime)
{
	bool bHasChanges = false;
	float Alpha = TransitionSpeed * DeltaTime;

	// Smooth position transition
	FVector CurrentPosition = GetCameraPosition();
	if (!CurrentPosition.Equals(TargetPosition, 1.0f))
	{
		FVector NewPosition = FMath::VInterpTo(CurrentPosition, TargetPosition, DeltaTime, TransitionSpeed);
		ApplyPosition(NewPosition);
		bHasChanges = true;

		if (CurrentPosition.Equals(TargetPosition, 1.0f))
		{
			OnCameraMoveEvent.Broadcast(TargetPosition);
		}
	}

	// Smooth rotation transition
	FRotator CurrentRot = GetCameraRotation();
	if (!CurrentRot.Equals(TargetRotation, 1.0f))
	{
		FRotator NewRotation = FMath::RInterpTo(CurrentRot, TargetRotation, DeltaTime, TransitionSpeed);
		ApplyRotation(NewRotation);
		bHasChanges = true;

		if (CurrentRot.Equals(TargetRotation, 1.0f))
		{
			OnCameraRotateEvent.Broadcast(TargetRotation);
		}
	}

	// Smooth zoom transition
	if (!FMath::IsNearlyEqual(CurrentZoomPercent, TargetZoom, 1.0f))
	{
		float NewZoom = FMath::FInterpTo(CurrentZoomPercent, TargetZoom, DeltaTime, TransitionSpeed);
		ApplyZoom(NewZoom);
		bHasChanges = true;

		if (FMath::IsNearlyEqual(CurrentZoomPercent, TargetZoom, 1.0f))
		{
			OnCameraZoomEvent.Broadcast(TargetZoom);
		}
	}
}