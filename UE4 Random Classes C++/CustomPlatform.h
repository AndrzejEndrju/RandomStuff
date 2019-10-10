#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Wall.generated.h"

class ALever;
class ATest_HUD;
class IObserver;

UCLASS(ClassGroup = (Custom))
class TEST_API ACustomPlatform : public AActor, public IObserver
{
	GENERATED_BODY()
	
public:	
	
	ACustomPlatform();

#pragma region Private Variables

private:

    /**
     * Variable that determines the timer which will call the reset method once the timer is finished.
     */
	FTimerHandle handle;

    /**
     * Variable that determines whether or not we can move the platform again.
     */
	bool CanMoveAgain : 1;

#pragma endregion

#pragma region Public Variables

protected:

    /**
     * Variable that determines whether or not the platform is moving.
     */
	UPROPERTY(BlueprintReadWrite)
	bool PlatformIsMoving;
	
    /**
     * Variable determines whether or not platform is using timer widget.
     */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool CanUseTimer;

    /**
     * Variable that determines time for player to perform action for example: switching all required levers in 10 seconds
     */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeForPlayerToPerformAction;

    /**
     * Variable that holds actor location from BeginPlay().
     */
	UPROPERTY(BlueprintReadWrite)
	FVector BaseLocation;

    /**
     * Time shown on timer(TimeIn by default equals Time but TimeIn is used in arythmetic operations)
     */
	UPROPERTY(BlueprintReadWrite)
	float TimeIn;

	UPROPERTY(BlueprintReadWrite)
	TWeakPtr<ATest_HUD*> HUD;

    /**
     * Array of levers necessary to perform an action on platform for example: moving platform.
     */
	UPROPERTY(EditAnywhere)
	TArray<ALever*> RequiredLevers;

    /**
     * Variable that determines whether or not we can move the platform after single usage.
     */
	UPROPERTY(BlueprintReadWrite)
	bool CanMoveAgain;

    /**
     * Amount of currently switched levers defined in RequiredLevers.
     */
	int32 AmountOfSwitchedLevers;

    /**
     * Variable that tells the platform if it has to move continuously from point BaseLocation to point EndLocation
     */
	UPROPERTY(EditAnywhere)
	bool IsPlatformMovement;

    /**
     * Variable that determines whether or not the gate can perform action after only one required lever switched or after every single one required lever switched.
     */
	UPROPERTY(EditAnywhere)
	bool CanStartTimerAfterAllInteracted;

    /**
     * Variable that determines where do we want to move the platform from BaseLoation.
     */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EndLocation;

    /**
     * Variable that determines whether or not we want the platform to stay permamently frozen after executing certain action.
     */
	UPROPERTY(EditAnywhere)
	bool bStopMovingAfterCertainAction;

#pragma endregion

#pragma region Events

    /**
     * Event that creates timer widget.
     */
	UFUNCTION(BlueprintImplementableEvent)
	void CreateTimerEvent();

    /**
     * Event that makes platform move continuously from point BaseLocation to point EndLocation.
     */
	UFUNCTION(BlueprintImplementableEvent)
	void PlatformMovementEvent();


	UFUNCTION(BlueprintNativeEvent)
	void Initialize(ALever* lever, bool CanWork);
	void Initialize_Implementation(ALever* lever, bool CanWork);

#pragma endregion

#pragma region Private Methods

private:

    /**
     * Method that checks if all required levers are switched.
     */
	bool CheckIfAllSwitched();

    /**
     * Method that resets platform(sets all variables to it's default state).
     */
	void Reset();

    /**
     * Execute basic platform check of lever object.
     */
	void Execute(ALever* LeverParam);

    /**
     * Method that updates levers with new IsWorking value.
     */
	void UpdateLevers(bool NewVal);

    /**
     * Method that hides timer widget.
     */
    void HideTimerWidget();

#pragma endregion

#pragma region Public Methods
public:

    /**
     * Method that starts the timer widget.
     */
    UFUNCTION(BlueprintCallable)
        void InstantTimer();

    /**
     * Method that subscribes selected levers to a platform.
     */
    UFUNCTION(BlueprintCallable)
        void SubscribeLevers();

#pragma endregion

#pragma region IObserver

    /**
     * Method updates platform with lever interact value and executes initialize.
     */
	virtual bool UpdateObserver_Implementation(ALever* lever, bool CanInteract) override;

#pragma endregion

protected:

	virtual void Tick(float DeltaSeconds) override;

	void BeginPlay() override;
};
