#include "PlatformMovementOnCollision.h"

UPlatformMovementOnCollision::UPlatformMovementOnCollision()
{
    PrimaryComponentTick.bCanEverTick = false;
    AnimationTimeInSeconds = 1;
}

void UPlatformMovementOnCollision::Initialize()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetComponentTickEnabled(true);
    SetBasicValues();
}

void UPlatformMovementOnCollision::SetBasicValues()
{
    Step = 0.f;
    Dir = 1;
    IsBaseLocationSet = false;
    CachedValueValueToGoDown = ValueToGoDown;
}

void UPlatformMovementOnCollision::BeginPlay()
{
    Super::BeginPlay();

    SetBasicValues();

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
    
    SetComponentTickEnabled(false);
}

void UPlatformMovementOnCollision::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    if (!IsBaseLocationSet)
    {
        BaseLocation = GetOwner()->GetActorLocation();
        IsBaseLocationSet = true;
    }

    CurrentLocation = GetOwner()->GetActorLocation();
    Step += ( DeltaTime / (AnimationTimeInSeconds / 2) ) * Dir;
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
            Dir = -1;
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
    }
}
