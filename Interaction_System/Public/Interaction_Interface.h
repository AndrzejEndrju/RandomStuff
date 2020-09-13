// Copyright Andrzej Serazetdinow, 2020 All Rights Reserved.

#pragma once

#include "InteractionLog.h"

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerInteractionComponent.h"
#include "InteractionInterface.generated.h"

UINTERFACE(MinimalAPI)
class UInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

class INTERACTIONSYSTEM_API IInteractionInterface
{
	GENERATED_BODY()

public:

	virtual uint8 GetPriority() const
	{
		return 0u;
	}

	virtual bool CanInteract(const AActor* Player)
	{
		return true;
	}

	virtual void Interact(UPlayerInteractionComponent* PIC)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Interacting without implementation for interaction."));
	}

	virtual void SubscribeToComponent(AActor* Player, bool CurrentlySelected)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Subscribing to player without implementation."));
	}
	 
	virtual void UnsubscribeFromComponent(AActor* Player)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Unsubscribing from player without implementation."));
	}

};
