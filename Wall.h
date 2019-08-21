#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Switch.h"
#include "Observer.h"
#include "UserWidget.h"
#include "Test_HUD.h"
#include "Public/Shared/Components/SequenceComponent.h"
#include "TestGameMode.h"
#include "Wall.generated.h"

UCLASS(ClassGroup = (Custom))
class TEST_API AWall : public AActor, public IObserver
{
	GENERATED_BODY()
	
public:	
	
	AWall();

#pragma region Private Variables
private:

    /**
     * Variable that determines the timer which will call the reset method once the timer is finished.
     */
	FTimerHandle handle;

    /**
     * Variable that determines whether or not we can open the wall again.
     */
	bool CanOpenAgain : 1;

#pragma endregion

#pragma region Public Variables
protected:

    /**
     * Variable that determines whether or not the wall is moving.
     */
	UPROPERTY(BlueprintReadWrite)
	bool WallIsMoving;

    /**
     * Variable that determines whether or not the wall can open during playing sequence.
     */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool CanOpenDuringSequence;

    /**
     * Variable determines whether or not wall is using timer widget.
     */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool CanUseTimer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence")
	USequenceComponent* Sequence;

    /**
     * Variable that determines time for player to perform action for example: switching all required switches, moving through the wall before it closes etc..
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
	ATest_HUD* HUD;

    /**
     * Array of switches needed to perform an action on wall for example: moving wall.
     */
	UPROPERTY(EditAnywhere)
	TArray<ASwitch*> RequiredSwitches;

    /**
     * Variable that determines whether or not we can open(move) the gate.
     */
	UPROPERTY(BlueprintReadWrite)
	bool CanOpenGate;

    /**
     * Variable that determines amount of currently active switches(switches from RequiredSwitches array that were switched).
     */
	UPROPERTY(BlueprintReadWrite)
	int32 AmountOfWorkingSwitches;

    /**
     * Variable that tells the gate if it has to move like a platform continuously from point BaseLocation to point EndLocation
     */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsPlatformMovement;

    /**
     * Variable that determines whether or not the gate can perform action after only one required switch switched or after every single one required switch switched.
     */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool CanStartTimerAfterAllInteracted;

    /**
     * Variable that determines where do we want to move the gate from BaseLoation.
     */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EndLocation;

    /**
     * Variable that determines whether or not we want the gate to stay permamently open after a certain action.
     */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPermamentOpen;

#pragma endregion

#pragma region Events

    /**
     * Event that creates timer widget.
     */
	UFUNCTION(BlueprintImplementableEvent)
	void CreateTimerEvent();

    /**
     * Event that opens(moves) the wall.
     */
	UFUNCTION(BlueprintImplementableEvent)
	void OpenWallEvent();

    /**
     * Event that makes wall move continuously from point BaseLocation to point EndLocation.
     */
	UFUNCTION(BlueprintImplementableEvent)
	void PlatformMovementEvent();

    /**
     * Event that executes equence.
     */
	UFUNCTION(BlueprintImplementableEvent)
	void SequenceEvent();

	UFUNCTION(BlueprintNativeEvent)
	void Initialize(ASwitch* Switcher, bool CanWork);
	void Initialize_Implementation(ASwitch* Switcher, bool CanWork);

#pragma endregion

#pragma region Private Methods
private:

    /**
     * Method that checks if all required switches were switched.
     */
	bool CheckIfAllSwitched();

    /**
     * Method that resets wall(sets all variables to it's default state).
     */
	void Reset();

    /**
     * Method that checks if all required switches were switched.
     */
	void Execute(ASwitch* SwitchVal);

    /**
     * Method that updates switches with new IsWorking value.
     */
	void UpdateSwitches(bool NewVal);

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
     * Method that subscribes selected switches to a wall.
     */
    UFUNCTION(BlueprintCallable)
        void SubscribeSwitches();

#pragma endregion

#pragma region IObserver

    /**
     * Method updates wall with switch interact value and executes initialize.
     */
	virtual bool UpdateObserver_Implementation(ASwitch* Switcher, bool CanInteract) override;

#pragma endregion

protected:

	virtual void Tick(float DeltaSeconds) override;

	void BeginPlay() override;
};
