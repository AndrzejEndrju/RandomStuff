// Copyright Andrzej Serazetdinow, 2020 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerInteractionComponent.generated.h"

class UInteractableComponent;
class UNameWidget;
class UInteractionHoldWidget;
class UInteractionWidgetOnInteractable;
class UInteractableWidget;

class UArrowComponent;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDynamicMulticastDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDynamicMulticastDelegateOP_I, AActor*, Interactable);

USTRUCT(BlueprintType)
struct FDebugStringProperties 
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Debug")
	FColor ValidTextColor = FColor::Green;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Debug")
	FColor InvalidTextColor = FColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Debug")
	float FontScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Debug")
	float HeightDifferenceInDebugStrings = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Debug")
	bool bDrawShadow = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Debug")
	bool bUseOwningActorLocationForDebugText = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Debug")
	bool bDrawDebugStringsByDefault = true;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class INTERACTIONSYSTEM_API UPlayerInteractionComponent final : public UActorComponent
{
	GENERATED_BODY()

private:

	TArray<TWeakObjectPtr<UInteractableComponent>> ActorsToInteract;

	// Name widget container
	TWeakObjectPtr<UNameWidget> InteractionWidgetName;

	// Interaction widget container
	TWeakObjectPtr<UInteractableWidget> InteractionWidgetBase;

	// Interaction widget on interactable container
	TWeakObjectPtr<UInteractionWidgetOnInteractable> InteractionWidgetOnInteractable;

	// Interaction Marker container
	TWeakObjectPtr<UUserWidget> InteractionMarker;

	// Interaction Progress Widget container
	TWeakObjectPtr<UInteractionHoldWidget> InteractionProgressWidget;

	// Player Pawn container
	TWeakObjectPtr<APawn> PWN;

	// Player Controller container
	TWeakObjectPtr<APlayerController> PC;

	float CurrentTimeInSecondsForButtonHold;

	bool IsInteracting : 1;

	bool IsOnlineInteracting : 1;

public:

	// Interactable currently interacted
	TWeakObjectPtr<UInteractableComponent> InteractableInteracted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Debug")
	FDebugStringProperties DSProperties;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	UArrowComponent* PlayerInteractableForwardVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	TSubclassOf<UNameWidget> NameWidgetBP;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	TSubclassOf<UInteractableWidget> InteractionWidgetBP;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	TSubclassOf<UInteractionWidgetOnInteractable> InteractionWidgetOnInteractableBP;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	TSubclassOf<UUserWidget> InteractableMarkerBP;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	TSubclassOf<UInteractionHoldWidget> InteractionProgresBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Option")
	bool bUseLowerPriorityFirst = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	bool bIsUsingFirstPersonMode = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bIsUsingFirstPersonMode"),
		Category = "Interaction")
	bool bPlayerHasToLookOnTheObjectInsteadOfCheckingAngle = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	bool CanSelectOnlyOneInteractable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	bool bShowOnlyOneInteractableName = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	bool CanShowSystemLog = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	bool bHideInteractableNameWhenPlayerCanInteract = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	bool bHideInteractionMarkerWhenPlayerCanInteract = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	bool bHideInteractableNameWhenInteractableIsUnreachable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	bool bHideInteractionMarkerWhenInteractableIsUnreachable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	bool bRotateWidgetsTowardsPlayerCamera = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	bool bRotateWidgetsTowardsPlayerPawn = false;

private:

	UPlayerInteractionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Set a player controller to show widgets on certain player's HUD
	void SetPC();

	void BeginPlay() override;

	void SortActors();

	void TryExecuteInteract(const TWeakObjectPtr<UInteractableComponent>& Actor);

	void ExecuteInteract(const TWeakObjectPtr<UInteractableComponent>& Actor);

#pragma region Interactable Name

private:

	void ShowInteractableName(TSubclassOf<UNameWidget>& WidgetClass, UInteractableComponent* Component);

public:

	void TryShowInteractableName(UInteractableComponent* Component);

	void TryHideInteractableName(UInteractableComponent* Component);
	
#pragma endregion

#pragma region Interaction Progress

private:

	void ShowInteractionProgress(TSubclassOf<UInteractionHoldWidget>& WidgetClass);

	void HideInteractionProgressWidget();

public:

	void TryShowInteractionProgress(UInteractableComponent* Component);

	void TryHideInteractionProgress(UInteractableComponent* Component);

#pragma endregion

#pragma region Interaction Widget On Interactable

private:

	void ShowInteractionWidgetOnInteractable(TSubclassOf<UInteractionWidgetOnInteractable>& WidgetClass,
		UInteractableComponent* Component);

public:

	void TryShowInteractionWidgetOnInteractable(UInteractableComponent* Component);

	void TryHideInteractionWidgetOnInteractable(UInteractableComponent* Component);

#pragma endregion

#pragma region Interactable Marker

private:

	void ShowInteractionMarker(TSubclassOf<UUserWidget>& WidgetClass, UInteractableComponent* Component);

public:

	void TryShowInteractionMarker(UInteractableComponent* Component);

	void TryHideInteractionMarker(UInteractableComponent* Component);

#pragma endregion

#pragma region Interaction Widget

private:

	void ShowInteractionWidget(const TSubclassOf<UInteractableWidget>& WidgetClass);

	void HideInteractionWidget();

public:

	void TryShowInteractionWidget(const UInteractableComponent* Component);

	void TryHideInteractionWidget(const UInteractableComponent* Component);

#pragma endregion

#pragma region Delegates

public:

	UPROPERTY(BlueprintAssignable, Category = "InteractionDelegates")
	FDynamicMulticastDelegateOP_I OnInteractableSubscribedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "InteractionDelegates")
	FDynamicMulticastDelegateOP_I OnInteractableUnsubscribedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "InteractionDelegates")
	FDynamicMulticastDelegateOP_I OnFirstInteractableSubscribedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "InteractionDelegates")
	FDynamicMulticastDelegateOP_I OnCanInteractDelegate;

	UPROPERTY(BlueprintAssignable, Category = "InteractionDelegates")
	FDynamicMulticastDelegate OnNoInteractablesLeftDelegate;

#pragma endregion

public:

	UFUNCTION(Server, Reliable, WithValidation)
	void InteractWithInteractablesOn_Server   (UInteractableComponent* ActorToInteract);

	// Used for button hold interaction to reset the CurrentTimeInSecondsForButtonHold variable
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void StopInteraction();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void InteractWithInteractablesOnServer();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void InteractWithInteractables();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool CanInteractWithAnyInteractable() const;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void AddActorToInteract(const AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void RemoveActorToInteract(const AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ChangeInteractionWidget(const TSubclassOf<UInteractableWidget>& WidgetClass);

};
