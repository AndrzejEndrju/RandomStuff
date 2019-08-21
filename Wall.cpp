#include "Wall.h"
#include "Switch.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Core/Public/Containers/Array.h"
#include "EngineUtils.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "TimerManager.h"

AWall::AWall()
{
	Sequence = CreateDefaultSubobject<USequenceComponent>("SequenceComponent");
	PrimaryActorTick.bCanEverTick = true;
	WallIsMoving = false;
	CanOpenAgain = true;
	AmountOfWorkingSwitches = 0;
}

void AWall::Initialize_Implementation(ASwitch* Switcher, bool bHasToResetWall)
{
	if (!bHasToResetWall)
	{
        Execute(Switcher);
	}
	else if(CanOpenAgain)
	{
		Reset();
        Execute(Switcher);
	}
}

void AWall::Execute(ASwitch* SwitchVal)
{
	for (ASwitch* Actor : RequiredSwitches)
	{
		if (SwitchVal == Actor)
		{
			AmountOfWorkingSwitches++;
			break;
		}
	}

    if (Sequence->CanPlaySequence())
    {
        SequenceEvent();
    }

	if (CanUseTimer)
	{
		if (CanStartTimerAfterAllInteracted)
		{
			CheckIfAllSwitched();
		}
        else
        {
            InstantTimer();
        }
	}
    else
    {
        CheckIfAllSwitched();
    }
}

void AWall::UpdateSwitches(bool NewVal)
{
	for (ASwitch* Switch : RequiredSwitches)
	{
		Switch->Execute_SetState(Switch, NewVal);
	}
}

void AWall::InstantTimer()
{
    if (Sequence->Played)
    {
        CreateTimerEvent();
    }

	WallIsMoving = true;
	SetActorTickEnabled(true);

	if (HUD->Timer)
	{
		CheckIfAllSwitched();
	}
	else
	{
		CheckIfAllSwitched();
		GetWorldTimerManager().SetTimer(handle, this, &AWall::Reset, TimeForPlayerToPerformAction, false);
	}
}

void AWall::SubscribeSwitches()
{
	for (ASwitch* Switch : RequiredSwitches)
	{
        if (Switch)
        {
            Switch->Execute_Subscribe(Switch, this);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("You have an undeclared required switch for %s actor"), *GetNameSafe(this));
        }
	}
}

void AWall::Reset()
{
    HideTimerWidget();
	AmountOfWorkingSwitches = 0;

	TimeIn = TimeForPlayerToPerformAction;
	WallIsMoving = false;
}

void AWall::HideTimerWidget()
{
    if (HUD->Timer)
    {
        HUD->HideTimerWidget();
    }
}

bool AWall::CheckIfAllSwitched()
{
	if (RequiredSwitches.Num() == AmountOfWorkingSwitches)
	{
        if (!bPermamentOpen && Sequence->Played && CanUseTimer)
        {
            CreateTimerEvent();
        }

		if (IsPlatformMovement && (Sequence->Played || CanOpenDuringSequence))
		{
			PlatformMovementEvent();
			CanOpenAgain = false;
			SetActorTickEnabled(false);
			UpdateSwitches(false);

            HideTimerWidget();
		}
		else
		{
			WallIsMoving = true;
			if (bPermamentOpen)
			{
				CanOpenAgain = false;
				UpdateSwitches(false);
                SetActorTickEnabled(false);
			}
            if (Sequence->Played || CanOpenDuringSequence)
            {
                UE_LOG(LogTemp, Warning, TEXT("%s is opening"), *GetNameSafe(this));
                OpenWallEvent();
                SetActorTickEnabled(true);
            }
		}
		return true;
	}
	return false;
}
bool AWall::UpdateObserver_Implementation(ASwitch* Switchero, bool CanInteract)
{
    Initialize_Implementation(Switchero, CanInteract);
	return true;
}

void AWall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (WallIsMoving && TimeIn > 0.f && Sequence->Played && CanUseTimer)
	{
		TimeIn -= DeltaTime;

        if (TimeIn <= 0)
        {
            Reset();
        }
	}
    else
    {
        WallIsMoving = false;
    }
}

void AWall::BeginPlay()
{
	Super::BeginPlay();

    SetActorTickEnabled(false);
	HUD = ATestGameMode::GetTestHud(GetWorld());
	BaseLocation = GetActorLocation();
	TimeIn = TimeForPlayerToPerformAction;

	SubscribeSwitches();
}
