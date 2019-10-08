// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FarmTaskHUD.generated.h"

class UPressButtonWidget;

UCLASS()
class AFarmTaskHUD : public AHUD
{
	GENERATED_BODY()
#pragma region Public Variables

public:

		UPROPERTY(BlueprintReadWrite)
		UPressButtonWidget* PressButtonWidget;

#pragma endregion

#pragma region Private Variables

private:

	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

#pragma endregion

#pragma region Public Methods

public:

	//Press Button Widget States
	void ShowPressButtonWidget();
	void HidePressButtonWidget();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

#pragma endregion

#pragma region Private Methods

private:

	void BeginPlay() override;

	AFarmTaskHUD();

#pragma endregion
};

