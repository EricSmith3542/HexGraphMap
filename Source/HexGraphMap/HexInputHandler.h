// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HexInputHandler.generated.h"

class AHexGraph;

/**
 * Input handling modes for different graph editing states
 */
UENUM(BlueprintType)
enum class EHexInputMode : uint8
{
	Normal		UMETA(DisplayName = "Normal"),
	LineDraw	UMETA(DisplayName = "Line Draw"),
	Fill		UMETA(DisplayName = "Fill"),
	Delete		UMETA(DisplayName = "Delete")
};

/**
 * Delegate signatures for input events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSelectEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeleteEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFillEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLineDrawStartEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLineDrawStopEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnZoomEvent, float, ZoomDelta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRotateEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMoveEvent, FVector2D, MovementVector);

/**
 * Manages input processing and state for the HexGraph system
 * Handles Enhanced Input integration and input mode management
 */
UCLASS(BlueprintType)
class HEXGRAPHMAP_API UHexInputHandler : public UObject
{
	GENERATED_BODY()

public:
	UHexInputHandler();

	/**
	 * Initialize the input handler with required dependencies
	 * @param InOwningHexGraph The HexGraph that owns this handler
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Input Handler")
	void Initialize(AHexGraph* InOwningHexGraph);

	/**
	 * Setup input component bindings
	 * @param InputComponent The input component to bind to
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Input Handler")
	void SetupInputBindings(UEnhancedInputComponent* InputComponent);

	/**
	 * Set the current input mode
	 * @param NewMode New input mode to set
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Input Handler")
	void SetInputMode(EHexInputMode NewMode);

	/**
	 * Get the current input mode
	 * @return Current input mode
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Input Handler")
	EHexInputMode GetInputMode() const { return CurrentInputMode; }

	/**
	 * Check if a specific input mode is active
	 * @param Mode Mode to check
	 * @return True if the mode is active
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Input Handler")
	bool IsInputModeActive(EHexInputMode Mode) const { return CurrentInputMode == Mode; }

	/**
	 * Enable or disable input processing
	 * @param bEnabled Whether input should be processed
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Input Handler")
	void SetInputEnabled(bool bEnabled) { bInputEnabled = bEnabled; }

	/**
	 * Check if input processing is enabled
	 * @return True if input is enabled
	 */
	UFUNCTION(BlueprintCallable, Category = "Hex Input Handler")
	bool IsInputEnabled() const { return bInputEnabled; }

	// Input Events
	UPROPERTY(BlueprintAssignable, Category = "Hex Input Events")
	FOnSelectEvent OnSelectEvent;

	UPROPERTY(BlueprintAssignable, Category = "Hex Input Events")
	FOnDeleteEvent OnDeleteEvent;

	UPROPERTY(BlueprintAssignable, Category = "Hex Input Events")
	FOnFillEvent OnFillEvent;

	UPROPERTY(BlueprintAssignable, Category = "Hex Input Events")
	FOnLineDrawStartEvent OnLineDrawStartEvent;

	UPROPERTY(BlueprintAssignable, Category = "Hex Input Events")
	FOnLineDrawStopEvent OnLineDrawStopEvent;

	UPROPERTY(BlueprintAssignable, Category = "Hex Input Events")
	FOnZoomEvent OnZoomEvent;

	UPROPERTY(BlueprintAssignable, Category = "Hex Input Events")
	FOnRotateEvent OnRotateEvent;

	UPROPERTY(BlueprintAssignable, Category = "Hex Input Events")
	FOnMoveEvent OnMoveEvent;

protected:
	/** Reference to the owning HexGraph */
	UPROPERTY()
	TObjectPtr<AHexGraph> OwningHexGraph;

	/** Current input mode */
	UPROPERTY(BlueprintReadOnly, Category = "Input State")
	EHexInputMode CurrentInputMode;

	/** Whether input processing is enabled */
	UPROPERTY(BlueprintReadOnly, Category = "Input State")
	bool bInputEnabled;

	/** Whether line drawing is currently active */
	UPROPERTY(BlueprintReadOnly, Category = "Input State")
	bool bLineDrawActive;

	/** Input action references */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> SelectAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> DeleteAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> FillAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> LineDrawAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> ZoomAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> RotateAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> MoveForwardAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> MoveBackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> MoveLeftAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> MoveRightAction;

private:
	// Input callback functions
	void HandleSelectInput();
	void HandleDeleteInput();
	void HandleFillInput();
	void HandleLineDrawStart();
	void HandleLineDrawOngoing();
	void HandleLineDrawStop();
	void HandleLineDrawCancel();
	void HandleZoomInput(const FInputActionValue& Value);
	void HandleRotateInput();
	void HandleMoveForwardInput(const FInputActionValue& Value);
	void HandleMoveBackInput(const FInputActionValue& Value);
	void HandleMoveLeftInput(const FInputActionValue& Value);
	void HandleMoveRightInput(const FInputActionValue& Value);

	/**
	 * Check if input should be processed based on current state
	 * @return True if input should be processed
	 */
	bool ShouldProcessInput() const;

	/**
	 * Load input actions from the owning HexGraph
	 */
	void LoadInputActionsFromHexGraph();

	/**
	 * Calculate movement vector from input value
	 * @param Value Input action value
	 * @param Direction Movement direction (Forward, Back, Left, Right)
	 * @return 2D movement vector
	 */
	FVector2D CalculateMovementVector(const FInputActionValue& Value, const FString& Direction) const;
};