#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Switch.generated.h"

class UInteractionComponent;
class ISubject;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableActor))
class TEST_API ALever : public AActor, public ISubject
{
    GENERATED_BODY()

#pragma region Private Variables

private:

    /**
     * If lever is used again after being used.
     */
    bool IsWorkingAgain : 1;

    /**
     * Observers that lever has to notify on state change.
     */
    UPROPERTY()
    TArray<AActor*> Observers;
    
    UInteractionComponent* InteractionComponent;

#pragma endregion

#pragma region Public Variables

public:

    /**
     * Variable that defines the priority value to interact higher numbers have less "priority".
     */
    UPROPERTY(EditAnywhere)
    int32 Priority;

#pragma endregion

#pragma region Public Methods

public:

    bool Interact();
    
    const bool CanInteract() const 
    { 
        return InteractionComponent->CanInteract; 
    };


#pragma endregion

#pragma region ISubject

    /**
     * @Param State - Flag that determines if player can interact with lever.
     */
    bool SetState_Implementation(bool State) override;

    /**
     * Method that subscribes observers to lever.
     * @Param Observer - Object that subscribes to lever
     */
    bool Subscribe_Implementation(AActor* Observer) override;

    /**
     * Notify observers.
     */
    bool Notify_Implementation() override;

#pragma endregion

#pragma region AActor

    void BeginPlay() override;

#pragma endregion
};
