// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class IInteractionInterface;
class AFarmTaskHUD;
class AFarmTaskPlayerController;
class AFarmTaskCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FARMTASK_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region Private Variables

private:	

	AFarmTaskHUD* HUD;

	AFarmTaskPlayerController* PC;

	TArray<IInteractionInterface*> Interactables;

#pragma endregion

#pragma region Public Methods

public:	

	//Initialize components variables
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Initialize(AFarmTaskCharacter* OwningPlayer);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void AddInteractable(AActor* Interactable);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void RemoveInteractable(AActor* Interactable);

	UFUNCTION(Category = "Interaction")
	void InteractWithInteractable();

#pragma endregion
};
