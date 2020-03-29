// Made by Andrzej Serazetdinow

#pragma once

#include "InteractionLog.h"

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interaction_Interface.generated.h"

UINTERFACE(MinimalAPI)
class UInteraction_Interface : public UInterface
{
	GENERATED_BODY()
};

class INTERACTION_SYSTEM_API IInteraction_Interface
{
	GENERATED_BODY()

public:

	virtual const uint8 GetPriority()
	{
		return 0;
	}

	virtual const bool CanInteract(AActor* Player)
	{
		return true;
	}

	virtual void Interact()
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Interacting without implementation for interaction."));
	}

	virtual void SubscribeToComponent(AActor* Player)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Subscribing to player without implementation."));
	}

	virtual void UnsubscribeFromComponent(AActor* Player)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Unsubscribing to player without implementation."));
	}

};
