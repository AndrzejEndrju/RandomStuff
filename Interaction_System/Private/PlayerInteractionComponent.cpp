// Copyright Andrzej Serazetdinow, 2020 All Rights Reserved.

#include "PlayerInteractionComponent.h"
#include "InteractionInterface.h"
#include "InteractableComponent.h"
#include "NameWidget.h"
#include "InteractionHoldWidget.h"
#include "InteractionWidgetOnInteractable.h"
#include "InteractableWidget.h"

#include "Blueprint/UserWidget.h"

#include "Components/WidgetComponent.h"
#include "Components/ArrowComponent.h"

#include "InteractionLog.h"

UPlayerInteractionComponent::UPlayerInteractionComponent()
	: CurrentTimeInSecondsForButtonHold(0.f), IsInteracting(false), IsOnlineInteracting(false)
{
	PlayerInteractableForwardVector = CreateDefaultSubobject<UArrowComponent>(FName("InteractableForwardVector"));

	SetIsReplicatedByDefault(true);

	PrimaryComponentTick.bCanEverTick = true;

	if (GetOwner())
	{
		PlayerInteractableForwardVector->AttachToComponent(GetOwner()->GetRootComponent(),
			FAttachmentTransformRules::KeepRelativeTransform);
	}
}

void UPlayerInteractionComponent::HideInteractionWidget()
{
	if (InteractionWidgetBase.IsValid() && InteractionWidgetBase.Get()->IsVisible())
	{
		InteractionWidgetBase->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPlayerInteractionComponent::HideInteractionProgressWidget()
{
	if (InteractionProgressWidget.IsValid())
	{
		InteractionProgressWidget->SetVisibility(ESlateVisibility::Hidden);
		CurrentTimeInSecondsForButtonHold = 0.f;
		InteractionProgressWidget->Reset();
	}
}

void UPlayerInteractionComponent::TryHideInteractableName(UInteractableComponent* Component)
{
	if (!Component)
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("Component passed to TryHideInteractableName() is nullptr."));
		return;
	}

	if (Component->InteractableName && Component->InteractableName->IsVisible())
	{
		Component->HideInteractableName();
	}
}

void UPlayerInteractionComponent::TryHideInteractionWidgetOnInteractable(UInteractableComponent* Component)
{
	if (!Component)
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("Component passed to TryHideInteractionWidgetOnInteractable() is nullptr."));
		return;
	}

	if (Component->InteractionWidgetOnInteractable && Component->InteractionWidgetOnInteractable->IsVisible())
	{
		Component->HideInteractionWidgetOnInteractable();
	}
}

void UPlayerInteractionComponent::TryHideInteractionProgress(UInteractableComponent* Component)
{
	if (!Component)
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("Component passed to TryHideInteractionProgress() is nullptr."));
		return;
	}

	if (Component->InteractionWidgetOnInteractable && Component->InteractionWidgetOnInteractable->IsVisible())
	{
		Component->HideInteractionWidgetOnInteractable();
	}
}

void UPlayerInteractionComponent::TryHideInteractionMarker(UInteractableComponent* Component)
{
	if (!Component)
	{
		UE_LOG(InteractionSystem, Warning, 
			TEXT("Component passed to TryHideInteractionMarker() is nullptr."));
		return;
	}

	if (Component->InteractionMarker && Component->InteractionMarker->IsVisible())
	{
		Component->HideInteractionMarker();
	}
}

void UPlayerInteractionComponent::TryHideInteractionWidget(const UInteractableComponent* Component)
{
	if (!Component)
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("Component passed to TryHideInteractionWidget() is nullptr."));
		return;
	}

	if (!CanInteractWithAnyInteractable() && InteractionWidgetBase.Get() && InteractionWidgetBase->IsVisible())
	{
		HideInteractionWidget();
	}
}

void UPlayerInteractionComponent::RemoveActorToInteract(const AActor* Actor)
{
	if (!Actor)
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("Actor passed to RemoveActorToInteract() is nullptr."));
		return;
	}

	if (!PWN.IsValid())
	{
		if (APawn* OWN = Cast<APawn>(GetOwner()))
		{
			PWN = OWN;
		}
		else
		{
			UE_LOG(InteractionSystem, Warning,
				TEXT("OWN assigned to GetOwner() is invalid is he inheriting from APawn? Error inside RemoveActorToInteract PlayerInteractionComponent. Owner name: %s"), *GetNameSafe(GetOwner()));
			return;
		}
	}

	if (!PWN.IsValid())
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("PWN assigned to GetOwner() is invalid weird error inside RemoveActorToInteract PlayerInteractionComponent. Owner name: %s"), *GetNameSafe(GetOwner()));
		return;
	}

	UInteractableComponent* Component = Actor->
		FindComponentByClass<UInteractableComponent>();

	if (!Component)
	{
		if (CanShowSystemLog)
		{
			UE_LOG(InteractionSystem, Log, 
				TEXT("Actor to interact does not contain UInteractableComponent component Therefore it won't be removed from ActorsToInteract for player %s."), *GetNameSafe(GetOwner()));
		}

		return;
	}

	if (ActorsToInteract.Contains(Component))
	{
		if (Component->AmountOfSubscribedPlayers)
		{
			Component->UnsubscribeFromComponent(GetOwner());
			TryHideInteractionMarker(Component);
			TryHideInteractionWidgetOnInteractable(Component);
			TryHideInteractableName(Component);

			if (InteractableInteracted.IsValid() &&
				InteractableInteracted.Get() == Component)
			{
				InteractableInteracted.Reset();
			}

			if (OnInteractableUnsubscribedDelegate.IsBound())
			{
				OnInteractableUnsubscribedDelegate.Broadcast(Component->GetOwner());
			}
		}

		ActorsToInteract.Remove(Component);

		StopInteraction();

		if (!ActorsToInteract.Num())
		{
			TryHideInteractionWidget(Component);

			if(OnNoInteractablesLeftDelegate.IsBound())
			{
				OnNoInteractablesLeftDelegate.Broadcast();
			}

			SetComponentTickEnabled(false);
		}
			
		if (CanShowSystemLog)
		{
			UE_LOG(InteractionSystem, Log, 
				TEXT("Removed %s from ActorsToInteract for %s player. Amount of actors to interact equals: %d"), *GetNameSafe(Actor), *GetNameSafe(GetOwner()), ActorsToInteract.Num());
		}
	}
}

void UPlayerInteractionComponent::ChangeInteractionWidget(const TSubclassOf<UInteractableWidget>& WidgetClass)
{
	if (WidgetClass)
	{
		ShowInteractionWidget(WidgetClass);
	}
}

bool UPlayerInteractionComponent::InteractWithInteractablesOn_Server_Validate(
	UInteractableComponent* ActorToInteract)
{
	return true;
}

void UPlayerInteractionComponent::InteractWithInteractablesOn_Server_Implementation(
	UInteractableComponent* ActorToInteract)
{
	if (!ActorToInteract)
	{
		return;
	}

	ExecuteInteract(ActorToInteract);
}

void UPlayerInteractionComponent::InteractWithInteractablesOnServer()
 {
	StopInteraction();
	IsOnlineInteracting = true;

	if (ActorsToInteract.Num() > 0)
	{
		if (CanSelectOnlyOneInteractable && InteractableInteracted.IsValid() && 
			InteractableInteracted.Get()->CanInteract(GetOwner()))
		{
			if (InteractableInteracted.Get()->InteractableStructure.bHoldButtonToInteract)
			{
				TryShowInteractionProgress(InteractableInteracted.Get());
				IsInteracting = true;
				SetComponentTickEnabled(true);
			}
			else
			{
				InteractWithInteractablesOn_Server(InteractableInteracted.Get());
				return;
			}
		}

		for (const auto& ActorToInteract : ActorsToInteract)
		{
			if (ActorToInteract->CanInteract(GetOwner()))
			{
				if (ActorToInteract.Get()->InteractableStructure.bHoldButtonToInteract)
				{
					InteractableInteracted = ActorToInteract;
					TryShowInteractionProgress(InteractableInteracted.Get());
					IsInteracting = true;
					SetComponentTickEnabled(true);
				}
				else
				{
					InteractWithInteractablesOn_Server(ActorToInteract.Get());
				}
			}
		}
	}
}

void UPlayerInteractionComponent::SortActors()
{
	ActorsToInteract.Sort([=](const TWeakObjectPtr<UInteractableComponent>& LHS,
		const TWeakObjectPtr<UInteractableComponent>& RHS)
	{
		return bUseLowerPriorityFirst ? LHS.Get()->GetPriority() < RHS.Get()->GetPriority()
			: LHS.Get()->GetPriority() > RHS.Get()->GetPriority();
	});
}

void UPlayerInteractionComponent::TryExecuteInteract(const TWeakObjectPtr<UInteractableComponent>& Actor)
{
	if (!Actor.IsValid())
	{
		return;
	}

	if (Actor.Get()->CanInteract(GetOwner()))
	{
		if (Actor.Get()->InteractableStructure.bHoldButtonToInteract)
		{
			StopInteraction();
			InteractableInteracted = Actor.Get();
			TryShowInteractionProgress(InteractableInteracted.Get());
			IsInteracting = true;
			SetComponentTickEnabled(true);
			return;
		}
		else
		{
			ExecuteInteract(Actor.Get());
			return;
		}
	}
}

void UPlayerInteractionComponent::InteractWithInteractables()
{
	if (ActorsToInteract.Num() > 0)
	{
		if (CanSelectOnlyOneInteractable && InteractableInteracted.IsValid())
		{
			if (!InteractableInteracted.Get()->CanInteract(GetOwner()))
			{
				StopInteraction();

				for (auto& Act : ActorsToInteract)
				{
					if (Act->CanInteract(GetOwner()))
					{
						InteractableInteracted = Act;
						TryExecuteInteract(InteractableInteracted.Get());
						return;
					}
				}
			}
			else
			{
				TryExecuteInteract(InteractableInteracted.Get());
			}

			return;
		}

		for (const auto& ActorToInteract : ActorsToInteract)
		{
			if(ActorToInteract->CanInteract(GetOwner()))
			{
				TryExecuteInteract(InteractableInteracted.Get());
				return;
			}
		}
	}
}

void UPlayerInteractionComponent::ExecuteInteract(const TWeakObjectPtr<UInteractableComponent>& Actor)
{
	if (!Actor.IsValid())
	{
		return;
	}

	if (Actor.Get()->InteractableStructure.bDisableAfterUsage)
	{
		Actor.Get()->Disable();
	}

	Actor.Get()->Interact(this);
}

void UPlayerInteractionComponent::ShowInteractableName(TSubclassOf<UNameWidget>& WidgetClass, 
	UInteractableComponent* Component)
{
	if (!Component)
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("Component passed to ShowInteractableName() is nullptr."));
		return;
	}

	if (!Component->InteractableName->IsVisible() && InteractionWidgetName.IsValid()
		&& Component->InteractableNameClass
		&& Component->InteractableNameClass->GetClass() == WidgetClass)
	{
		return;
	}

	if (Component->InteractableNameClass 
		&& Component->InteractableNameClass->GetClass() != WidgetClass
		&& PC.IsValid()
		&& PC->IsLocalPlayerController())
	{
		InteractionWidgetName = CreateWidget<UNameWidget>(PC.Get(), WidgetClass);

		if (InteractionWidgetName.IsValid())
		{
			Component->ShowInteractableName(InteractionWidgetName.Get());

			if (Component->InteractableName)
			{
				Component->InteractableNameClass = WidgetClass;
			}
		}
	}
	else
	{
		if (!PC.IsValid())
		{
			SetPC();
		}

		if (PC.IsValid() && PC->IsLocalPlayerController())
		{
			InteractionWidgetName = CreateWidget<UNameWidget>(PC.Get(), WidgetClass);

			if (InteractionWidgetName.IsValid())
			{
				Component->ShowInteractableName(InteractionWidgetName.Get());
			}
		}

	}
}

void UPlayerInteractionComponent::ShowInteractionWidgetOnInteractable(
	TSubclassOf<UInteractionWidgetOnInteractable>& WidgetClass,
	UInteractableComponent* Component)
{
	if (!Component)
	{
		UE_LOG(InteractionSystem, Warning, 
			TEXT("Component passed to ShowInteractionWidgetOnInteractable() is nullptr."));
		return;
	}

	if ((bHideInteractionMarkerWhenPlayerCanInteract 
		|| bHideInteractableNameWhenPlayerCanInteract)
		&& InteractionWidgetOnInteractable.IsValid())
	{
		if (bHideInteractionMarkerWhenPlayerCanInteract)
		{
			Component->HideInteractionMarker();
		}

		if (bHideInteractableNameWhenPlayerCanInteract)
		{
			Component->HideInteractableName();
		}

		Component->ShowInteractionWidgetOnInteractable(InteractionWidgetOnInteractable.Get());

		return;
	}

	if (Component->InteractionWidgetOnInteractable && !Component->InteractionWidgetOnInteractable->IsVisible() 
		&& InteractionWidgetOnInteractable.IsValid()
		&& Component->InteractionWidgetOnInteractableClass &&
		Component->InteractionWidgetOnInteractableClass->
		GetClass() == WidgetClass)
	{
		Component->ShowInteractionWidgetOnInteractable(InteractionWidgetOnInteractable.Get());

		return;
	}
	 
	if (Component->InteractionWidgetOnInteractableClass && 
		Component->InteractionWidgetOnInteractableClass->
		GetClass() != WidgetClass && PC.IsValid() && PC->IsLocalPlayerController())
	{
		InteractionWidgetOnInteractable = CreateWidget<UInteractionWidgetOnInteractable>(PC.Get(), WidgetClass);

		if (InteractionWidgetOnInteractable.IsValid())
		{
			Component->ShowInteractionWidgetOnInteractable(InteractionWidgetOnInteractable.Get());
		}
	}
	else
	{
		if (!PC.IsValid())
		{
			SetPC();
		}

		if (PC.IsValid() && PC->IsLocalPlayerController())
		{
			InteractionWidgetOnInteractable = CreateWidget<UInteractionWidgetOnInteractable>(PC.Get(), WidgetClass);

			if (InteractionWidgetOnInteractable.IsValid())
			{
				if (bHideInteractionMarkerWhenPlayerCanInteract)
				{
					Component->HideInteractionMarker();
				}

				if (bHideInteractableNameWhenPlayerCanInteract)
				{
					Component->HideInteractableName();
				}

				Component->ShowInteractionWidgetOnInteractable(InteractionWidgetOnInteractable.Get());
			}
		}
	}
}

void UPlayerInteractionComponent::ShowInteractionProgress(
	TSubclassOf<UInteractionHoldWidget>& WidgetClass)
{
	if (InteractionProgressWidget.IsValid() && InteractionProgressWidget->GetClass() != WidgetClass && PC.IsValid() &&
		PC->IsLocalPlayerController())
	{
		InteractionProgressWidget = CreateWidget<UInteractionHoldWidget>(PC.Get(), WidgetClass,
			TEXT("InteractableProgressWidget"));

		if (InteractionProgressWidget.IsValid())
		{
			InteractionProgressWidget->AddToViewport();
		}
	}
	else if (InteractionProgressWidget.IsValid() && InteractionProgressWidget->GetVisibility() ==
		ESlateVisibility::Hidden)
	{
		InteractionProgressWidget->SetVisibility(ESlateVisibility::Visible);
		return;
	}
	else
	{
		if (!PC.IsValid())
		{
			SetPC();
		}

		if (PC.IsValid() && PC->IsLocalPlayerController())
		{
			InteractionProgressWidget = CreateWidget<UInteractionHoldWidget>(PC.Get(), WidgetClass,
				TEXT("InteractableProgressWidget"));

			if (InteractionProgressWidget.IsValid())
			{
				InteractionProgressWidget->AddToViewport();
				InteractionProgressWidget->SetVisibility(ESlateVisibility::Visible); //Just to make sure it's Visible
			}
		}
	}
}

void UPlayerInteractionComponent::ShowInteractionMarker(TSubclassOf<UUserWidget>& WidgetClass, 
	UInteractableComponent* Component)
{
	if (!Component)
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("Component passed to ShowInteractionMarker() is nullptr."));
		return;
	}

	if (Component->InteractionMarker && !Component->InteractionMarker->IsVisible() 
		&& InteractionMarker.IsValid() && Component->InteractableMarkerClass
		&& Component->InteractableMarkerClass->GetClass() == WidgetClass)
	{
		Component->ShowInteractionMarker(InteractionMarker.Get());

		return;
	}

	if (Component->InteractableMarkerClass &&
		Component->InteractableMarkerClass->GetClass() != WidgetClass 
		&& PC.IsValid() && PC->IsLocalPlayerController())
	{

		InteractionMarker = CreateWidget<UUserWidget>(PC.Get(), WidgetClass, TEXT("InteractableMarker"));

		if (InteractionMarker.IsValid())
		{
			Component->ShowInteractionMarker(InteractionMarker.Get());

			if (Component->InteractionMarker)
			{
				Component->InteractableMarkerClass = WidgetClass;
			}
		}
	}
	else
	{
		if (!PC.IsValid())
		{
			SetPC();
		}

		if (PC.IsValid() && PC->IsLocalPlayerController())
		{
			InteractionMarker = CreateWidget<UUserWidget>(PC.Get(), WidgetClass, TEXT("InteractableMarker"));

			if (InteractionMarker.IsValid())
			{
				Component->ShowInteractionMarker(InteractionMarker.Get());
			}
		}
	}
}

void UPlayerInteractionComponent::ShowInteractionWidget(const TSubclassOf<UInteractableWidget>& WidgetClass)
{
	if (InteractionWidgetBase.IsValid() && InteractionWidgetBase->GetClass() != WidgetClass && PC.IsValid() &&
		PC->IsLocalPlayerController())
	{
		InteractionWidgetBase = CreateWidget<UInteractableWidget>(PC.Get(), WidgetClass,
			TEXT("InteractableWidget"));

		if (InteractionWidgetBase.IsValid())
		{
			if (InteractableInteracted.IsValid())
			{
				InteractionWidgetBase->OnTextChanged(
					InteractableInteracted.Get()->InteractableStructure.InteractionText);
			}
			InteractionWidgetBase->AddToViewport();
		}
	}
	else if (InteractionWidgetBase.IsValid())
	{
		InteractionWidgetBase->OnTextChanged(
			InteractableInteracted.Get()->InteractableStructure.InteractionText);

		if (InteractionWidgetBase.Get()->Visibility == ESlateVisibility::Hidden)
		{
			InteractionWidgetBase->SetVisibility(ESlateVisibility::Visible);
		}

		return;
	}
	else
	{
		if (!PC.IsValid())
		{
			SetPC();
		}

		if (PC.IsValid() && PC->IsLocalPlayerController())
		{
			InteractionWidgetBase = CreateWidget<UInteractableWidget>(PC.Get(), WidgetClass,
				TEXT("InteractableWidget"));

			if (InteractionWidgetBase.IsValid())
			{
				if (InteractableInteracted.IsValid())
				{
					InteractionWidgetBase->OnTextChanged(
						InteractableInteracted.Get()->InteractableStructure.InteractionText);
				}
				InteractionWidgetBase->AddToViewport();
				InteractionWidgetBase->SetVisibility(ESlateVisibility::Visible); //Just to make sure it's Visible
			}
		}
	}
}

void UPlayerInteractionComponent::SetPC()
{
	if (PC.IsValid())
	{
		return;
	}

	if (!PWN.IsValid())
	{
		PWN = Cast<APawn>(GetOwner());
	}

	if (PWN.IsValid())
	{
		PC = Cast<APlayerController>(PWN->GetController());
	}
	else
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("PWN assigned to GetOwner() is invalid weird error inside SetPC PlayerInteractionComponent. Owner name: %s"), *GetNameSafe(GetOwner()));
	}
}

bool UPlayerInteractionComponent::CanInteractWithAnyInteractable() const
{
	for (auto& ActorToInteract : ActorsToInteract)
	{
		if (ActorToInteract->CanInteract(GetOwner()))
		{
			return true;
		}
	}

	return false;
}

void UPlayerInteractionComponent::TryShowInteractableName(UInteractableComponent* Component)
{
	if (!Component)
	{
		UE_LOG(InteractionSystem, Warning, 
			TEXT("Component passed to TryShowInteractableName() is nullptr."));
		return;
	}

	if (Component->InteractableStructure.bDisabled || 
		(bHideInteractableNameWhenPlayerCanInteract && Component->CanInteract(GetOwner())))
	{
		return;
	}

	if (NameWidgetBP)
	{
		ShowInteractableName(NameWidgetBP, Component);
	}
}

void UPlayerInteractionComponent::TryShowInteractionWidgetOnInteractable(
	UInteractableComponent* Component)
{
	if (!Component)
	{
		UE_LOG(InteractionSystem, Warning, 
			TEXT("Component passed to TryShowInteractionWidgetOnInteractable() is nullptr."));
		return;
	}

	if (Component->InteractableStructure.bDisabled)
	{
		return;
	}

	if (InteractionWidgetOnInteractableBP)
	{
		ShowInteractionWidgetOnInteractable(InteractionWidgetOnInteractableBP, Component);
	}
}

void UPlayerInteractionComponent::TryShowInteractionProgress(
	UInteractableComponent* Component)
{
	if (!Component)
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("Component passed to TryShowInteractionProgress() is nullptr."));
		return;
	}

	if (Component->InteractableStructure.bDisabled)
	{
		return;
	}

	if (InteractionProgresBP)
	{
		ShowInteractionProgress(InteractionProgresBP);
	}
}
 
void UPlayerInteractionComponent::TryShowInteractionMarker(UInteractableComponent* Component)
{
	if (!Component)
	{
		UE_LOG(InteractionSystem, Warning, 
			TEXT("Component passed to TryShowInteractionMarker() is nullptr."));
		return;
	}

	if (Component->InteractableStructure.bDisabled || (bHideInteractionMarkerWhenPlayerCanInteract &&
		Component->CanInteract(GetOwner())))
	{
		return;
	}

	if (InteractableMarkerBP)
	{
		ShowInteractionMarker(InteractableMarkerBP, Component);
	}
}

void UPlayerInteractionComponent::TryShowInteractionWidget(const UInteractableComponent* Component)
{
	if (!Component)
	{
		UE_LOG(InteractionSystem, Warning, 
			TEXT("Component passed to TryShowInteractionWidget() is nullptr."));
		return;
	}

	if (Component->InteractableStructure.bDisabled)
	{
		return;
	}

	if (InteractionWidgetBP)
	{
		ShowInteractionWidget(InteractionWidgetBP);
	}
}

void UPlayerInteractionComponent::AddActorToInteract(const AActor* Actor)
{
	if (!Actor)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Actor passed to AddActorToInteract() is nullptr."));
		return;
	}

	if (!PWN.IsValid())
	{
		if (APawn* OWN = Cast<APawn>(GetOwner()))
		{
			PWN = OWN;
		}
		else
		{
			UE_LOG(InteractionSystem, Warning,
				TEXT("OWN assigned to GetOwner() is invalid is he inheriting from APawn? Error inside AddActorToInteract PlayerInteractionComponent. Owner name: %s"), *GetNameSafe(GetOwner()));
			return;
		}
	}

	UInteractableComponent* Component = Actor->
		FindComponentByClass<UInteractableComponent>();

	if (!Component)
	{
		UE_LOG(InteractionSystem, Warning, 
			TEXT("Actor passed to AddActorToInteract() has nullptr interactable component."));
		return;
	}

	if (ActorsToInteract.Contains(Component))
	{
		return;
	}

	Component->SubscribeToComponent(GetOwner(), !InteractableInteracted.IsValid());

	if (!Component->IsSubscribed(this))
	{
		return;
	}

	ActorsToInteract.Add(Component);
	Component->InstancedDSP = DSProperties;

	if (!InteractableInteracted.IsValid())
	{
		StopInteraction();
		InteractableInteracted = Component;
	}

	SortActors();

	Component->SetWidgetRotationSettings(bRotateWidgetsTowardsPlayerCamera,
		bRotateWidgetsTowardsPlayerPawn);

	if (ActorsToInteract.Num() == 1)
	{
		if (OnFirstInteractableSubscribedDelegate.IsBound())
		{
			OnFirstInteractableSubscribedDelegate.Broadcast(Component->GetOwner());
		}
	}

	if (OnInteractableSubscribedDelegate.IsBound())
	{
		OnInteractableSubscribedDelegate.Broadcast(Component->GetOwner());
	}

	if (CanShowSystemLog)
	{
		UE_LOG(InteractionSystem, Log, 
			TEXT("Added %s to ActorsToInteract for %s player. Amount of actors to interact equals: %d"), *GetNameSafe(Actor), *GetNameSafe(GetOwner()), ActorsToInteract.Num());
	}
}

void UPlayerInteractionComponent::StopInteraction()
{
	IsInteracting = false;
	CurrentTimeInSecondsForButtonHold = 0.f;
	HideInteractionProgressWidget();
	SetComponentTickEnabled(false);
}

void UPlayerInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	SetComponentTickEnabled(false);
}

void UPlayerInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!InteractableInteracted.IsValid())
	{
		StopInteraction();

		for (auto& Act : ActorsToInteract)
		{
			if (Act->CanInteract(GetOwner()))
			{
				InteractableInteracted = Act;
				break;
			}
		}
	}
	else if (!InteractableInteracted.Get()->CanInteract(GetOwner()) || !IsInteracting)
	{
		InteractableInteracted.Reset();
		StopInteraction();
		return;
	}

	if (!InteractionProgressWidget.IsValid() || !InteractableInteracted.IsValid())
	{
		StopInteraction();
		return;
	}

	if (InteractableInteracted.Get()->InteractableStructure.TimeInSecondsForButtonHold
		<= CurrentTimeInSecondsForButtonHold)
	{
		IsOnlineInteracting ? InteractWithInteractablesOn_Server(InteractableInteracted.Get()) 
			: ExecuteInteract(InteractableInteracted.Get());

		if (InteractableInteracted.IsValid() && InteractableInteracted.Get()->InteractableStructure.CanHoldMultipleTimes)
		{
			CurrentTimeInSecondsForButtonHold = 0.f;
		}
		else
		{
			StopInteraction();
		}
	}
	else
	{
		CurrentTimeInSecondsForButtonHold += DeltaTime;
		InteractionProgressWidget.Get()->OnHoldCalled(CurrentTimeInSecondsForButtonHold / InteractableInteracted.Get()->InteractableStructure.TimeInSecondsForButtonHold);
	}
}
