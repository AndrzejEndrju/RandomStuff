#include "lever.h"
#include "TestGameMode.h"

ASwitch::ALever()
{
    Sequence = CreateDefaultSubobject<USequenceComponent>("SequenceComponent");
}

void ALever::TryPlaySequence()
{
    if (IsPlayingSequece && Sequence->CanPlaySequence())
    {
        Sequence->PlaySequence();
    }
}

void ALever::BeginPlay()
{
    Super::BeginPlay();
    
    IsWorkingAgain = false;
}

bool ALever::Interact()
{
    TryPlaySequence();
    Notify_Implementation();
    return true;
}

bool ALever::Subscribe_Implementation(AActor* Observer)
{
    TScriptInterface<IObserver> Observe = Observer;
    if (Observe)
    {
        Observers.Add(Observer);
        InteractionComponent->CanInteract = true;
        return true;
    }
    return false;
}

bool ALever::Notify_Implementation()
{
    if (Observers.Num() > 0)
    {
        for (const auto& Observer : Observers)
        {
            TScriptInterface<IObserver> Observe = Observer;
            if (Observe)
            {
                Observing->Execute_UpdateObserver(Observer, this, IsWorkingAgain);
                IsWorkingAgain = true;
            }
        }
        return true;
    }
    return false;
}

bool ALever::SetState_Implementation(bool State)
{
    InteractionComponent->CanInteract = State;
    Notify_Implementation();
    return State;
}
