#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Subject.h"
#include "Public/Shared/Components/SequenceComponent.h"
#include "ColorMode.h"
#include "Public/Gameplay/Interactions/InteractableComponent.h"
#include "Public/Gameplay/Interactions/InteractInteractableComponent.h"

#include "Switch.generated.h"

class UInteraction;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableActor))
class TEST_API ASwitch : public AActor, public ISubject
{
    GENERATED_BODY()

#pragma region Private Variables

private:
    ASwitch();

    /**
     * Variable that defines whether or not a certain switch is working again.
     */
    bool IsWorkingAgain : 1;

    /**
     * Observers that a certain switch has to notify whenever his state changes.
     */
    UPROPERTY()
    TArray<AActor*> Observers;


#pragma endregion

#pragma region Public Variables
public:

    UPROPERTY(EditAnywhere)
    UUseInteractableComponent* InteractableComponent;

    /**
     * Variable that defines whether or not the sequence should be played.
     */
    UPROPERTY(EditAnywhere)
    bool IsPlayingSequece;

    /**
     * Variable that defines whether or not we want to disable player input during sequence.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool CanDisablePlayerInput;

    /**
     * Variable that defines the priority value to interact for interaction system higher numbers have less priority.
     */
    UPROPERTY(EditAnywhere)
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence")
    USequenceComponent* Sequence;

#pragma endregion

#pragma region Private Methods
private:
    /**
     * Method that checks if player choosed to play a sequence.
     */
    void TryPlaySequence();

#pragma endregion

#pragma region Public Methods
public:

    /**
     * Method that runs sequence in blueprint.
     */
    UFUNCTION(BlueprintImplementableEvent)
    void PlaySequence();

    /**
     * Method that determines what do we want to do when player interacts with switch.
     */
    UFUNCTION(BlueprintCallable)
    bool Interact();

    /**
     * Method that checks if player can interact with a certain switch.
     */
    FORCEINLINE bool CanInteract() const { return InteractableComponent->CanInteract; };


#pragma endregion

#pragma region ISubject

    /**
     * Method that set's a state of a certain switch.
     * @Param State - Flag that determines whether or not switch can interact
     */
    bool SetState_Implementation(bool State) override;

    /**
     * Method that subscribes observers to switch.
     * @Param Observer - Object that subscribes as observer to switch
     */
    bool Subscribe_Implementation(AActor* Observer) override;

    /**
     * Method that notify's observers.
     */
    bool Notify_Implementation() override;

#pragma endregion

#pragma region AActor

    void BeginPlay() override;

#pragma endregion
};
