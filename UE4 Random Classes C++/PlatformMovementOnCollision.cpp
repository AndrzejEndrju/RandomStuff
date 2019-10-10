#include "PlatformMovementOnCollision.h"

UPlatformMovementOnCollision::UPlatformMovementOnCollision()
{
    PrimaryComponentTick.bCanEverTick = true;
    IsFinishedMoving = true;
    AnimationTimeInSeconds = 1.f;
}

void UPlatformMovementOnCollision::Initialize()
{
    SetComponentTickEnabled(true);
    IsFinishedMoving = false;
    SetBasicValues();
}

void UPlatformMovementOnCollision::SetBasicValues()
{
    Direction = 1;

    if (IsFinishedMoving)
    {
        IsBaseLocationSet = false;
    }
}

void UPlatformMovementOnCollision::BeginPlay()
{
    SetComponentTickEnabled(false);

    Super::BeginPlay();

    SetBasicValues();

    CachedValueValueToGoDown = ValueToGoDown;
    AnimationTimeInSeconds /= 2;

    if (AnimationTimeInSeconds <= 0.f)
    {
        UE_LOG(LogTemp, Error, TEXT("AnimationTimeInSeconds is less or equal to 0 in UPlatformMovementOnCollision component in %s"), GetOwner());
        PrimaryComponentTick.bCanEverTick = false;
    }

    if (!Curve)
    {
        UE_LOG(LogTemp, Error, TEXT("Curve is not set in UPlatformMovementOnCollision component in %s"), GetOwner());
        PrimaryComponentTick.bCanEverTick = false;
    }
}

void UPlatformMovementOnCollision::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    if (!IsBaseLocationSet)
    {
        BaseLocation = GetOwner()->GetActorLocation();
        IsBaseLocationSet = true;
    }

    CurrentLocation = GetOwner()->GetActorLocation();
    Step += ( DeltaTime / ( AnimationTimeInSeconds / 2 ) ) * Direction;
    ValueToGoDown = CachedValueValueToGoDown * Step;

    if (Step > 0.f)
    {
        GetOwner()->SetActorLocation(
            FMath::Lerp(
                CurrentLocation,
                FVector(
                    CurrentLocation.X,
                    CurrentLocation.Y,
                    BaseLocation.Z - ValueToGoDown
                ),
                Curve->GetFloatValue(Step)
            )
        );

        if (Step >= 1.f)
        {
            Step = 1.f;
            Direction = -1;
        }
    }
    else if (Step < 0.f)
    {
        SetBasicValues();
        GetOwner()->SetActorLocation(
            FVector(
                CurrentLocation.X,
                CurrentLocation.Y, 
                BaseLocation.Z - ValueToGoDown
            )
        );
        SetComponentTickEnabled(false);
        IsFinishedMoving = true;
    }
}
