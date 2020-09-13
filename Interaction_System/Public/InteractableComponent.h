// Copyright Andrzej Serazetdinow, 2020 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "InteractionInterface.h"
#include "PlayerInteractionComponent.h"

#include "InteractableComponent.generated.h"

class UWidgetComponent;
class USphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDynamicMulticastDelegateOP_P, AActor*, Player);

constexpr float PlayerLooksAtInteractableValue = 3.14f;
constexpr float Multiplier = 180.f / PI;
constexpr float FAILED_Angle = 400.f;

USTRUCT(BlueprintType)
struct FInteractable
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Option")
	FText InteractableName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Option")
	FText InteractionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Option")
	bool bAlwaysDrawDebugStrings = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bDoesDistanceToPlayerMatter"),
		Category = "Interactable Option")
	float MaximumDistanceToPlayer = 125.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bDoesAngleMatter"),
		Category = "Interactable Option")
	float PlayersAngleMarginOfErrorToInteractable = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Option")
	bool bHoldButtonToInteract = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Option")
	bool CanHoldMultipleTimes = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bHoldButtonToInteract"),
		Category = "Interactable Option")
	float TimeInSecondsForButtonHold = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bRandomizePriority"),
		Category = "Interactable Option")
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bRandomizePriority"),
		Category = "Interactable Option")
	int32 PriorityRandomizedMIN = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bRandomizePriority"),
		Category = "Interactable Option")
	int32 PriorityRandomizedMAX = 255;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Option")
	bool bRandomizePriority = false;

	// Disable interaction option with this object after usage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Option")
	bool bDisableAfterUsage = false;

	/*If true the interactable will always check if player can reach to interactable (interactable is not behind wall etc), if false
	the interactable will ignore the reach to interactable.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Option")
	bool bHasToBeReacheable = true;

	/*If true the interactable will always check if distance to player is right, if false
	the interactable will ignore the distance to player.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Option")
	bool bDoesDistanceToPlayerMatter = true;

	/*If true the interactable will always check if player's angle to interactable is right, if false
	the interactable will ignore the player's angle to interactable.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Option")
	bool bDoesAngleMatter = true;

	/*If true the interactable won't work unless you change this value to false inside your
	C++ class or blueprint.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Option")
	bool bDisabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Debug Options")
	bool bOverrideDebugStringLocation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Debug Options")
	bool bDrawDebugLineForReachability = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bOverrideDebugStringLocation"),
		Category = "Interactable Debug Options")
	FVector NewDebugStringLocation;

};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class INTERACTIONSYSTEM_API UInteractableComponent : public USceneComponent, public IInteractionInterface
{
	GENERATED_BODY()

private:

	TArray<TWeakObjectPtr<UPlayerInteractionComponent>> PlayerComponents;

	FTimerHandle InteractionTimerHandle;

	FRotator WidgetRotation;

	bool bCanBroadcastCanInteract : 1;

	bool bRotateWidgetsTowardsCamera : 1;

	bool bRotateWidgetsTowardsPlayerPawnCMP : 1;

	bool InteractionWidgetOnInteractableUsable : 1;

	bool InteractionMarkerUsable : 1;

	bool NameWidgetUsable : 1;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NameWidget")
	UWidgetComponent* InteractableName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractableMarker")
	UWidgetComponent* InteractionMarker;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractionWidgetOnInteractable")
	UWidgetComponent* InteractionWidgetOnInteractable;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction")
	USphereComponent* SphereComponent;

	TSubclassOf<UNameWidget> InteractableNameClass;

	TSubclassOf<UUserWidget> InteractableMarkerClass;

	TSubclassOf<UInteractionWidgetOnInteractable> InteractionWidgetOnInteractableClass;

	FDebugStringProperties InstancedDSP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!RandomizeRarityValue"),
		Category = "Interaction")
	int32 RarityValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool RandomizeRarityValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "RandomizeRarityValue"),
		Category = "Interaction")
	int32 RarityRandomizedMIN = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "RandomizeRarityValue"),
		Category = "Interaction")
	int32 RarityRandomizedMAX = 255;

	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	TArray<AActor*> SubscribedPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Interaction")
	FInteractable InteractableStructure;

	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	int32 AmountOfSubscribedPlayers = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bUseRotationVariablesFromPlayerComponent = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bUseRotationVariablesFromPlayerComponent"),
		Category = "Interaction")
	bool bRotateWidgetsTowardsPlayerCamera = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bUseRotationVariablesFromPlayerComponent"),
		Category = "Interaction")
	bool bRotateWidgetsTowardsPlayerPawn = false;

	bool CanShowInteractionMarker;

#pragma region Delegates

public:

	UPROPERTY(BlueprintAssignable)
	FDynamicMulticastDelegateOP_P InteractDelegate;

	UPROPERTY(BlueprintAssignable)
	FDynamicMulticastDelegateOP_P OnCanInteractDelegate;

	UPROPERTY(BlueprintAssignable)
	FDynamicMulticastDelegateOP_P OnSubscribedDelegate;

	UPROPERTY(BlueprintAssignable)
	FDynamicMulticastDelegateOP_P OnSelectedDelegate;

	UPROPERTY(BlueprintAssignable)
	FDynamicMulticastDelegateOP_P OnUnsubscribedDelegate;

	UPROPERTY(BlueprintAssignable)
	FDynamicMulticastDelegate OnNameWidgetUsable;

	UPROPERTY(BlueprintAssignable)
	FDynamicMulticastDelegate OnInteractionWidgetOnInteractableUsable;

	UPROPERTY(BlueprintAssignable)
	FDynamicMulticastDelegate OnInteractionMarkerUsable;

#pragma endregion

#pragma region Interaction Interface

public:

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual bool CanInteract(const AActor* Player) override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void Interact(UPlayerInteractionComponent* PIC) override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void SubscribeToComponent(AActor* Player, bool CurrentlySelected) override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void UnsubscribeFromComponent(AActor* Player) override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual uint8 GetPriority() const override
	{
		return InteractableStructure.Priority;
	}

#pragma endregion

public:

	UFUNCTION(BlueprintCallable, Category = "NameWidget")
	void ShowInteractableName(UNameWidget* Widget);

	UFUNCTION(BlueprintCallable, Category = "NameWidget")
	void HideInteractableName();

	UFUNCTION(BlueprintCallable, Category = "InteractionWidgetOnInteractable")
	void ShowInteractionWidgetOnInteractable(UInteractionWidgetOnInteractable* Widget);

	UFUNCTION(BlueprintCallable, Category = "InteractionWidgetOnInteractable")
	void HideInteractionWidgetOnInteractable();

	UFUNCTION(BlueprintCallable, Category = "InteractionMarker")
	void ShowInteractionMarker(UUserWidget* Widget);

	UFUNCTION(BlueprintCallable, Category = "InteractionMarker")
	void HideInteractionMarker();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool IsAnySubscribedPlayerLocallyControlled();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool CanAnyPlayerInteract();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	APawn* GetLocallyControlledPlayer() const;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void CheckOverlappingActors();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Enable();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Disable();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetWidgetRotationSettings(bool IsCameraRotation, bool IsPawnRotation);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool IsSubscribed(const UPlayerInteractionComponent* PlayerComponent) const;

private:

	UInteractableComponent();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	void BroadcastCanInteract(const UPlayerInteractionComponent* PlayerComponent) const;

	void RotateWidgetsToPlayer(bool ToCamera);

	void DrawDebugStrings(const AActor* Player) const;

	bool CheckReachability(const AActor* SubscribedPlayer) const;

	float CheckDistanceToPlayer(const AActor* SubscribedPlayer) const;

	float CheckAngleToPlayer(const AActor* SubscribedPlayer) const;

	void TryHideWidgets(UPlayerInteractionComponent* PlayerComponent);

protected:

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};
