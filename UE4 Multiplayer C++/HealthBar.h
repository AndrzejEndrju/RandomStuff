// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBar.generated.h"

/**
 * 
 */
UCLASS()
class FARMTASK_API UHealthBar : public UUserWidget
{
	GENERATED_BODY()

#pragma region Public Varriables

public:

	UFUNCTION(BlueprintCallable)
	void InitializeHealthValues(const float MaxHealthVal, float HealthVal);

	UFUNCTION(BlueprintCallable)
	void OnDamageTaken();

	UPROPERTY(BlueprintReadWrite)
	float MaxHealth;

	UPROPERTY(BlueprintReadWrite)
	float Health;

#pragma endregion
	
};
