// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FarmTaskPlayerController.generated.h"

class AFarmTaskCharacter;

/**
 * 
 */
UCLASS()
class FARMTASK_API AFarmTaskPlayerController : public APlayerController
{
	GENERATED_BODY()

	AFarmTaskPlayerController();

#pragma region Public Methods

public:

	/** Fires a projectile. */
	UFUNCTION(BlueprintCallable)
	void TryFire();

	/** Interaction. */
	UFUNCTION(BlueprintCallable)
	void TryInteract();

	/** Makes Player Jump */
	UFUNCTION(BlueprintCallable)
	void TryJump();

	/** Stops Player Jumping */
	UFUNCTION(BlueprintCallable)
	void TryStopJumping();

	/** Handles moving forward/backward */
	UFUNCTION(BlueprintCallable)
	void TryMoveForward(const float Val);

	/** Handles stafing movement, left and right */
	UFUNCTION(BlueprintCallable)
	void TryMoveRight(const float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	UFUNCTION(BlueprintCallable)
	void TryTurnAtRate(const float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	UFUNCTION(BlueprintCallable)
	void TryLookUpAtRate(const float Rate);

#pragma endregion

#pragma region Private Methods

	void BeginPlay() override;

#pragma endregion

#pragma region Private Variables

private:

	AFarmTaskCharacter* Character;
	
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY()
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY()
	float BaseLookUpRate;

#pragma endregion
};
