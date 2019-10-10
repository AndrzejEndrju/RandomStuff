#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Runtime/Engine/Classes/Curves/CurveFloat.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "Runtime/Core/Public/Math/UnrealMathVectorCommon.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "PlatformMovementOnCollision.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TEST_API UPlatformMovementOnCollision : public UActorComponent
{
    GENERATED_BODY()

    UPlatformMovementOnCollision();

#pragma region Private Variables
private:

    /**
     * Variable used to store Z location of the owner when player jumps on it.
     */
    FVector BaseLocation;

    /**
     * Variable used to store current X, Y, Z owner location.
     */
    FVector CurrentLocation;

    /**
     * Variable used to store value of variable ValueToGoDown.
     */
    float CachedValueValueToGoDown;

    /**
     * Variable used to get a float value from curve asset.
     */
    float Step;

    /**
     * Variable that determines whether or not the base location of owner is set.
     */
    bool IsBaseLocationSet : 1;

    /**
     * Variable that determines whether or not the movement of the platform is finished.
     */
    bool IsFinishedMoving : 1;

#pragma endregion

#pragma region Public Variables
public:

    /**
     * Variable that determines time required to complete a movement animation.
     */
    UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "20.0"))
    float AnimationTimeInSeconds;

    /**
     * Variable that determines the direction of movement.
     */
    UPROPERTY(BlueprintReadWrite)
    int32 Direction;

    /**
     * Variable that determines the value that will be decremented from Z value of platform position for movement animation.
     */
    UPROPERTY(EditAnywhere)
    float ValueToGoDown;

    /**
     * Variable that stores curve asset.
     */
    UPROPERTY(EditAnywhere)
    UCurveFloat* Curve;

#pragma endregion

#pragma region Private Methods
private:

    /**
     * Method that sets basic values for this class.
     */
    void SetBasicValues();

#pragma endregion

#pragma region Public Methods
public:

    /**
     * Method that initializes this class with event tick and calls SetBasicValues whenever player interact with platform collision(logic done in blueprint on overlap->initialize).
     */
    UFUNCTION(BlueprintCallable)
    void Initialize();

#pragma endregion

#pragma region Private Events
private:

    virtual void BeginPlay() override;

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma endregion

};
