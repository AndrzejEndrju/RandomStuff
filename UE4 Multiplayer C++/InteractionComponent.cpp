// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionComponent.h"
#include "FarmTaskHUD.h"
#include "GameFramework/Actor.h"
#include "FarmTaskPlayerController.h"
#include "InteractionInterface.h"
#include "FarmTaskCharacter.h"

void UInteractionComponent::Initialize(AFarmTaskCharacter* OwningPlayer)
{
	PC = Cast<AFarmTaskPlayerController>(OwningPlayer->Controller);
	
	if (PC)
	{
		HUD = Cast<AFarmTaskHUD>(PC->GetHUD());
	}
}

void UInteractionComponent::AddInteractable(AActor* Interactable)
{
	IInteractionInterface* asInteractable = Cast<IInteractionInterface>(Interactable);
	if (asInteractable)
	{
		Interactables.Add(asInteractable);

		if (HUD)
		{
			HUD->ShowPressButtonWidget();
		}
	}
}

void UInteractionComponent::RemoveInteractable(AActor* Interactable)
{
	IInteractionInterface* asInteractable = Cast<IInteractionInterface>(Interactable);
	if (asInteractable)
	{
		if (Interactables.Contains(asInteractable))
		{
			Interactables.Pop(asInteractable);
			
			if (Interactables.Num() == 0 && HUD)
			{
				HUD->HidePressButtonWidget();
			}
		}
	}
}

void UInteractionComponent::InteractWithInteractable()
{
	if (Interactables.Num() > 0)
	{
		Interactables.Sort(
			[](const IInteractionInterface& Actor1, const IInteractionInterface& Actor2)
			{ return Actor1.GetPriority() < Actor2.GetPriority(); }
		);

		IInteractionInterface* asInteractable = Cast<IInteractionInterface>(Interactables[0]);
		if (asInteractable)
		{
			if (asInteractable->CanInteract())
			{
				asInteractable->Interact();
			}
		}
		
	}
}
