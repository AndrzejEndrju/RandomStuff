#include "CustomPlatform.h"
#include "lever.h"
#include "Observer.h"
#include "TestGameMode.h"
#include "TestHud.h"
#include "TimerManager.h"

ACustomPlatform::ACustomPlatform()
{
	PrimaryActorTick.bCanEverTick = true;
	PlatformIsMoving = false;
	CanMoveAgain = true;
	AmountOfSwitchedLevers = 0;
}

void ACustomPlatform::Initialize_Implementation(ALever* lever, bool CanResetPlatforml)
{
	if (!CanResetPlatforml)
	{
       		Execute(lever);
	}
	else if(CanMoveAgain)
	{
		Reset();
        	Execute(lever);
	}
}

void ACustomPlatform::Execute(ALever* LeverParam)
{
	for (ALever* lever : RequiredLevers)
	{
		if (LeverParam == lever)
		{
			AmountOfSwitchedLevers++;
			break;
		}
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

void ACustomPlatform::UpdateSwitches(bool NewVal)
{
	for (ALever* lever : RequiredLevers)
	{
		lever->Execute_SetState(lever, NewVal);
	}
}

void ACustomPlatform::InstantTimer()
{
	PlatformIsMoving = true;
	SetActorTickEnabled(true);

	if (HUD->Timer)
	{
		CheckIfAllSwitched();
	}
	else
	{
		CheckIfAllSwitched();
		GetWorldTimerManager().SetTimer(handle, this, &ACustomPlatform::Reset, TimeForPlayerToPerformAction, false);
	}
}

void ACustomPlatform::SubscribeLevers()
{
	for (ALever* lever : RequiredLevers)
	{
		if (lever)
		{
		    lever->Execute_Subscribe(lever, this);
		}
		else
		{
		    UE_LOG(LogTemp, Warning, TEXT("You have an undeclared required lever for %s actor"), *GetNameSafe(this));
		}
	}
}

void ACustomPlatform::Reset()
{
   	HideTimerWidget();
	AmountOfSwitchedLevers = 0;

	TimeIn = TimeForPlayerToPerformAction;
	PlatformIsMoving = false;
}

void ACustomPlatform::HideTimerWidget()
{
    if (HUD->Timer)
    {
        HUD->HideTimerWidget();
    }
}

bool ACustomPlatform::CheckIfAllSwitched()
{
	if (RequiredLevers.Num() == AmountOfSwitchedLevers)
	{
		if (!bPermamentOpen && CanUseTimer)
		{
		    CreateTimerEvent();
		}

		if (IsPlatformMovement)
		{
			PlatformMovementEvent();
			CanMoveAgain = false;
			SetActorTickEnabled(false);
			UpdateLevers(false);

           		HideTimerWidget();
		}
		else
		{
			PlatformIsMoving = true;
			if (bStopMovingAfterCertainAction)
			{
				CanMoveAgain = false;
				UpdateLevers(false);
               			SetActorTickEnabled(false);
			}
		}
		return true;
	}
	return false;
}
bool ACustomPlatform::UpdateObserver_Implementation(ALever* lever, bool CanInteract)
{
	Initialize_Implementation(lever, CanInteract);
	return true;
}

void ACustomPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PlatformIsMoving && TimeIn > 0.f && CanUseTimer)
	{
		TimeIn -= DeltaTime;

		if (TimeIn <= 0)
		{
		    Reset();
		}
	}
	else
	{
		PlatformIsMoving = false;
	}
}

void ACustomPlatform::BeginPlay()
{
	Super::BeginPlay();

	SetActorTickEnabled(false);
	HUD = ATestGameMode::GetTestHud(GetWorld());
	BaseLocation = GetActorLocation();
	TimeIn = TimeForPlayerToPerformAction;

	SubscribeLevers();
}
