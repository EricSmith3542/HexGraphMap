// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Math/Vector.h"
#include "Math/Vector2D.h"
#include "Math/Rotator.h"
#include "HexCameraController.generated.h"

class AHexGraph;

/**
 * Camera movement modes for different interaction styles
 */
UENUM(BlueprintType)
enum class EHexCameraMode : uint8
{
	Free		UMETA(DisplayName = "Free Movement"),
	Locked		UMETA(DisplayName = "Locked to Graph"),
	Follow		UMETA(DisplayName = "Follow Target")
};

/**
 * Delegate signatures for camera events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraZoomEvent, float, NewZoomPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraRotateEvent, FRotator, NewRotation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraMoveEvent, FVector, NewLocation);

/**
 * Manages camera movement, zoom, rotation, and positioning for the HexGraph system
 * Provides smooth transitions and configurable camera behavior
 */
UCLASS(BlueprintType)
class HEXGRAPHMAP_API UHexCameraController : public UObject
{
	GENERATED_BODY()

public:
	UHexCameraController();

	/**
	 * Initialize the camera controller with required components
	 * @param InOwningHexGraph The HexGraph that owns this controller
	 * @param InSpringArm The spring arm component to control
	 * @param InCamera The camera component to control
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Camera Controller")
	void Initialize(AHexGraph* InOwningHexGraph, USpringArmComponent* InSpringArm, UCameraComponent* InCamera);

	/**
	 * Process zoom input
	 * @param ZoomDelta Change in zoom (positive = zoom in, negative = zoom out)
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Camera Controller")
	void ProcessZoom(float ZoomDelta);

	/**
	 * Process rotation input using mouse delta
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Camera Controller")
	void ProcessRotation();

	/**
	 * Process movement input
	 * @param MovementVector 2D movement vector (X = forward/back, Y = left/right)
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Camera Controller")
	void ProcessMovement(const FVector2D& MovementVector);

	/**
	 * Set camera position directly
	 * @param NewPosition New world position for the camera
	 * @param bUseSmoothing Whether to smooth the transition
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Camera Controller")
	void SetCameraPosition(const FVector& NewPosition, bool bUseSmoothing = true);

	/**
	 * Set camera rotation directly
	 * @param NewRotation New rotation for the camera
	 * @param bUseSmoothing Whether to smooth the transition
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Camera Controller")
	void SetCameraRotation(const FRotator& NewRotation, bool bUseSmoothing = true);

	/**
	 * Set zoom level directly
	 * @param NewZoomPercent New zoom percentage (100 = default)
	 * @param bUseSmoothing Whether to smooth the transition
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Camera Controller")
	void SetZoomLevel(float NewZoomPercent, bool bUseSmoothing = true);

	/**
	 * Get current zoom percentage
	 * @return Current zoom level
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Camera Controller")
	float GetZoomLevel() const { return CurrentZoomPercent; }

	/**
	 * Get current camera position
	 * @return Current world position of the camera system
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Camera Controller")
	FVector GetCameraPosition() const;

	/**
	 * Get current camera rotation
	 * @return Current rotation of the camera system
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Camera Controller")
	FRotator GetCameraRotation() const;

	/**
	 * Set camera movement mode
	 * @param NewMode New camera mode
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Camera Controller")
	void SetCameraMode(EHexCameraMode NewMode);

	/**
	 * Get current camera mode
	 * @return Current camera mode
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Camera Controller")
	EHexCameraMode GetCameraMode() const { return CurrentCameraMode; }

	/**
	 * Focus camera on a specific world position
	 * @param TargetPosition World position to focus on
	 * @param bUseSmoothing Whether to smooth the transition
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Camera Controller")
	void FocusOnPosition(const FVector& TargetPosition, bool bUseSmoothing = true);

	/**
	 * Reset camera to default position and settings
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Camera Controller")
	void ResetToDefaults();

	/**
	 * Update camera controller (called each frame)
	 * @param DeltaTime Time since last update
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Camera Controller")
	void UpdateCamera(float DeltaTime);

	// Camera Events
	UPROPERTY(BlueprintAssignable, Category = "Camera Events")
	FOnCameraZoomEvent OnCameraZoomEvent;

	UPROPERTY(BlueprintAssignable, Category = "Camera Events")
	FOnCameraRotateEvent OnCameraRotateEvent;

	UPROPERTY(BlueprintAssignable, Category = "Camera Events")
	FOnCameraMoveEvent OnCameraMoveEvent;

protected:
	/** Reference to the owning HexGraph */
	UPROPERTY()
	TObjectPtr<AHexGraph> OwningHexGraph;

	/** Spring arm component reference */
	UPROPERTY()
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	/** Camera component reference */
	UPROPERTY()
	TObjectPtr<UCameraComponent> CameraComponent;

	/** Current camera mode */
	UPROPERTY(BlueprintReadOnly, Category = "Camera State")
	EHexCameraMode CurrentCameraMode;

	/** Current zoom percentage */
	UPROPERTY(BlueprintReadOnly, Category = "Camera State")
	float CurrentZoomPercent;

	/** Current pan speed */
	UPROPERTY(BlueprintReadOnly, Category = "Camera State")
	float CurrentPanSpeed;

	/** Current rotation */
	UPROPERTY(BlueprintReadOnly, Category = "Camera State")
	FRotator CurrentRotation;

	/** Target position for smooth transitions */
	UPROPERTY()
	FVector TargetPosition;

	/** Target rotation for smooth transitions */
	UPROPERTY()
	FRotator TargetRotation;

	/** Target zoom for smooth transitions */
	UPROPERTY()
	float TargetZoom;

	/** Whether smooth transitions are enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	bool bUseSmoothTransitions;

	/** Speed of smooth transitions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float TransitionSpeed;

	/** Zoom limits */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float MinZoomPercent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float MaxZoomPercent;

	/** Movement boundaries (if using locked mode) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	FVector MovementBoundsMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	FVector MovementBoundsMax;

	/** Whether to enforce movement boundaries */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	bool bEnforceMovementBounds;

private:
	/**
	 * Apply zoom to camera components
	 * @param ZoomPercent Zoom percentage to apply
	 */
	void ApplyZoom(float ZoomPercent);

	/**
	 * Apply rotation to camera components
	 * @param Rotation Rotation to apply
	 */
	void ApplyRotation(const FRotator& Rotation);

	/**
	 * Apply position to camera components
	 * @param Position Position to apply
	 */
	void ApplyPosition(const FVector& Position);

	/**
	 * Clamp position to movement bounds if enabled
	 * @param Position Position to clamp
	 * @return Clamped position
	 */
	FVector ClampToBounds(const FVector& Position) const;

	/**
	 * Load camera settings from HexGraph settings
	 */
	void LoadCameraSettings();

	/**
	 * Update smooth transitions
	 * @param DeltaTime Time since last update
	 */
	void UpdateSmoothTransitions(float DeltaTime);
};