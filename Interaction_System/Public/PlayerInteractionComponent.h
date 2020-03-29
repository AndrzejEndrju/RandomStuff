// Made by Andrzej Serazetdinow

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerInteractionComponent.generated.h"

class UInteractableComponent;
class USphereComponent;
class UArrowComponent;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDynamicMulticastDelegate);


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
class INTERACTION_SYSTEM_API UPlayerInteractionComponent final : public UActorComponent
{
	GENERATED_BODY()

private:

	TArray<UInteractableComponent*> ActorsToInteract;

	// Interaction widget container
	UUserWidget* InteractionWidgetBlueprint;

	// Interaction widget on interactable container
	UUserWidget* InteractionWidgetOnInteractable;

	// Interaction Marker container
	UUserWidget* InteractionMarker;

	// Player Controller container
	APlayerController* PC;

	// Player Pawn container
	APawn* PWN;

	bool IsInteractionWidgetHidden;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Debug")
	FDebugStringProperties DSProperties;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	USphereComponent* SphereComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	UArrowComponent* PlayerInteractableForwardVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	TSubclassOf<UUserWidget> InteractionWidgetBP;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	TSubclassOf<UUserWidget> InteractionWidgetOnInteractableBP;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	TSubclassOf<UUserWidget> InteractableMarkerBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Option")
	bool bUseLowerPriorityFirst = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	bool bIsUsingFirstPersonMode = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bIsUsingFirstPersonMode"), Category = "Interaction")
	bool bPlayerHasToLookOnTheObjectInsteadOfCheckingAngle = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	bool CanShowSystemLog = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	bool bHideInteractionMarkerWhenPlayerCanInteract = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	bool bRotateWidgetsTowardsPlayerCamera = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	bool bRotateWidgetsTowardsPlayerPawn = false;

private:

	void BeginPlay() override;

	UPlayerInteractionComponent();

	// Set a player controller to show widgets on certain player's HUD
	void SetPC();

#pragma region Interaction Widget On Interactable

private:

	void ShowInteractionWidgetOnInteractable(TSubclassOf<UUserWidget>& WidgetClass, UInteractableComponent* Component);

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

	void ShowInteractionWidget(TSubclassOf<UUserWidget>& WidgetClass);

	void HideInteractionWidget();

public:

	void TryShowInteractionWidget(UInteractableComponent* Component);

	void TryHideInteractionWidget(UInteractableComponent* Component);

#pragma endregion

#pragma region Delegates

public:

	UPROPERTY(BlueprintAssignable, Category = "InteractionDelegates")
	FDynamicMulticastDelegate OnInteractableSubscribedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "InteractionDelegates")
	FDynamicMulticastDelegate OnInteractableUnsubscribedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "InteractionDelegates")
	FDynamicMulticastDelegate OnFirstInteractableSubscribedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "InteractionDelegates")
	FDynamicMulticastDelegate OnNoInteractablesLeftDelegate;

	UPROPERTY(BlueprintAssignable, Category = "InteractionDelegates")
	FDynamicMulticastDelegate OnCanInteractDelegate;

#pragma endregion

public:

	UFUNCTION(BlueprintCallable, Category = "Interaction", Server, Reliable, WithValidation)
	void InteractWithInteractables_Server(UInteractableComponent* ActorToInteract);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void InteractWithInteractables();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool CanInteractWithAnyInteractable();

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void AddActorToInteract(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void RemoveActorToInteract(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ChangeInteractionWidget(TSubclassOf<UUserWidget>& WidgetClass);

};
