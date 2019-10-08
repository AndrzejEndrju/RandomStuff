// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractionInterface.generated.h"

UINTERFACE(MinimalAPI)
class UInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FARMTASK_API IInteractionInterface
{
	GENERATED_BODY()

public:

	virtual bool Interact() = 0;

	virtual const bool CanInteract() const = 0;

	virtual const int GetPriority() const = 0;
};
