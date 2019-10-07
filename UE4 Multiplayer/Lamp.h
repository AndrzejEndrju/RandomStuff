// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractionInterface.h"
#include "GameFramework/Actor.h"
#include "Lamp.generated.h"

class UPointLightComponent;

UCLASS()
class FARMTASK_API ALamp : public AActor, public IInteractionInterface
{
	GENERATED_BODY()
	
#pragma region Public Variables

public:	

	UPROPERTY(EditAnywhere, Replicated)
	UPointLightComponent* Light;

	UPROPERTY(ReplicatedUsing = OnRep_IsLightOn)
	bool IsLightOn;

	UPROPERTY(EditAnywhere)
	int InteractionPriority = 0;

#pragma endregion

#pragma region Public Methods

public:

	const bool CanInteract() const override
	{
		return true;
	}

	const int GetPriority() const override
	{
		return InteractionPriority;
	}

	//Change light state
	UFUNCTION()
	void OnRep_IsLightOn();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	bool Interact() override;

#pragma endregion


#pragma region Private Methods

private:

	ALamp();

#pragma endregion

};
