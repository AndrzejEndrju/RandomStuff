// Made by Andrzej Serazetdinow

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Interaction_Interface.h"
#include "PlayerInteractionComponent.h"
#include "InteractableComponent.generated.h"

class USphereComponent;
class UWidgetComponent;

constexpr float PlayerLooksAtInteractableValue = 3.14f;
constexpr float Multiplier = 180.f / PI;
constexpr float FAILED_Angle = 400.f;

USTRUCT(BlueprintType)
struct FInteractable
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Option")
	bool bAlwaysDrawDebugStrings = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bDoesDistanceToPlayerMatter"), Category = "Interactable Option")
	float MaximumDistanceToPlayer = 125.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bDoesAngleMatter"), Category = "Interactable Option")
	float PlayersAngleMarginOfErrorToInteractable = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bRandomizePriority"), Category = "Interactable Option")
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bRandomizePriority"), Category = "Interactable Option")
	int32 PriorityRandomizedMIN = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bRandomizePriority"), Category = "Interactable Option")
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Option")
	bool bChangeEnabilityOnlyLocally = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Debug Options")
	bool bOverrideDebugStringLocation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Debug Options")
	bool bDrawDebugLineForReachability = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bOverrideDebugStringLocation"), Category = "Interactable Debug Options")
	FVector NewDebugStringLocation;

};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class INTERACTION_SYSTEM_API UInteractableComponent : public USceneComponent, public IInteraction_Interface
{
	GENERATED_BODY()

private:

	TArray<UPlayerInteractionComponent*> PlayerComponents;

	FRotator WidgetRotation;

	bool bCanBroadcastCanInteract : 1;

	bool bRotateWidgetsTowardsCamera : 1;

	bool bRotateWidgetsTowardsPlayerPawnCMP : 1;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractableMarker")
	UWidgetComponent* InteractionMarker;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractionWidgetOnInteractable")
	UWidgetComponent* InteractionWidgetOnInteractable;

	TSubclassOf<UUserWidget> InteractableMarkerClass;

	TSubclassOf<UUserWidget> InteractionWidgetOnInteractableClass;

	FDebugStringProperties InstancedDSP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!RandomizeRarityValue"), Category = "Interaction")
	int32 RarityValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool RandomizeRarityValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "RandomizeRarityValue"), Category = "Interaction")
	int32 RarityRandomizedMIN = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "RandomizeRarityValue"), Category = "Interaction")
	int32 RarityRandomizedMAX = 255;

	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	TArray<AActor*> SubscribedPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Interaction")
	FInteractable InteractableStructure;

	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	int32 AmountOfSubscribedPlayers = 0;

	bool IsInteractionWidgetOnInteractableHidden : 1;

	bool IsInteractionMarkerHidden : 1;

	bool CanShowInteractionMarker : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bUseRotationVariablesFromPlayerComponent = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bUseRotationVariablesFromPlayerComponent"), Category = "Interaction")
	bool bRotateWidgetsTowardsPlayerCamera = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bUseRotationVariablesFromPlayerComponent"), Category = "Interaction")
	bool bRotateWidgetsTowardsPlayerPawn = false;

#pragma region Delegates

public:

	UPROPERTY(BlueprintAssignable)
	FDynamicMulticastDelegate InteractDelegate;

	UPROPERTY(BlueprintAssignable)
	FDynamicMulticastDelegate OnCanInteractDelegate;

	UPROPERTY(BlueprintAssignable)
	FDynamicMulticastDelegate OnSubscribedDelegate;

	UPROPERTY(BlueprintAssignable)
	FDynamicMulticastDelegate OnUnsubscribedDelegate;

	UPROPERTY(BlueprintAssignable)
	FDynamicMulticastDelegate OnInteractionWidgetOnInteractableUsable;

	UPROPERTY(BlueprintAssignable)
	FDynamicMulticastDelegate OnInteractionMarkerUsable;

#pragma endregion

public:

	UFUNCTION(BlueprintCallable, Category = "InteractionWidgetOnInteractable")
	void ShowInteractionWidgetOnInteractable(UUserWidget* Widget);

	UFUNCTION(BlueprintCallable, Category = "InteractionWidgetOnInteractable")
	void HideInteractionWidgetOnInteractable();

	UFUNCTION(BlueprintCallable, Category = "InteractionMarker")
	void ShowInteractionMarker(UUserWidget* Widget);

	UFUNCTION(BlueprintCallable, Category = "InteractionMarker")
	void HideInteractionMarker();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SubscribeToComponent(AActor* Player) override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void UnsubscribeFromComponent(AActor* Player) override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	const uint8 GetPriority() override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool IsAnySubscribedPlayerLocallyControlled();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool CanAnyPlayerInteract();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	APawn* GetLocallyControlledPlayer();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	const bool CanInteract(AActor* Player) override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact() override;

	// Enable an disabled interactable for interaction on client
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Enable();

	// Disable an enabled interactable for interaction on client
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Disable(bool bDisableForEveryone);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetWidgetRotationSettings(bool IsCameraRotation, bool IsPawnRotation);

private:

	UInteractableComponent();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	void BroadcastCanInteract(UPlayerInteractionComponent* PlayerComponent);

	void RotateWidgetsToPlayerCamera();

	void RotateWidgetsToPlayerPawn();

	void DrawDebugStrings(AActor* Player);

	bool CheckReachability(AActor* SubscribedPlayer) const;

	const float CheckDistanceToPlayer(AActor* SubscribedPlayer) const;

	const float CheckAngleToPlayer(AActor* SubscribedPlayer) const;

protected:

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};
