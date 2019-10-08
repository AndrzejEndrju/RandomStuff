#include "Switch.h"
#include "TestGameMode.h"
#include "Utilities.h"

ASwitch::ASwitch()
{
    Sequence = CreateDefaultSubobject<USequenceComponent>("SequenceComponent");
}

void ASwitch::TryPlaySequence()
{
    if (IsPlayingSequece && Sequence->CanPlaySequence())
    {
        InteractableComponent->CanInteract = false;
        PlaySequence();
    }
}

void ASwitch::BeginPlay()
{
    IsWorkingAgain = false;
    Super::BeginPlay();

    UUtilities::GetFirstComponentOfClass(this, InteractableComponent);
}

bool ASwitch::Interact()
{
    TryPlaySequence();
    Notify_Implementation();
    return true;
}

bool ASwitch::Subscribe_Implementation(AActor* Observer)
{
    TScriptInterface<IObserver> Observing = Observer;
    if (Observing)
    {
        Observers.Add(Observer);
        InteractableComponent->CanInteract = true;
        return true;
    }
    return false;
}

bool ASwitch::Notify_Implementation()
{
    if (Observers.Num() > 0)
    {
        for (AActor* Observer : Observers)
        {
            TScriptInterface<IObserver> Observing = Observer;
            if (Observing)
            {
                Observing->Execute_UpdateObserver(Observer, this, IsWorkingAgain);
                IsWorkingAgain = true;
            }
        }
        return true;
    }
    return false;
}

bool ASwitch::SetState_Implementation(bool State)
{
    InteractableComponent->CanInteract = State;
    Notify_Implementation();
    return State;
}
