// Fill out your copyright notice in the Description page of Project Settings.


#include "FarmTaskPlayerController.h"
#include "FarmTaskCharacter.h"
#include "Kismet/GameplayStatics.h"

AFarmTaskPlayerController::AFarmTaskPlayerController()
{
	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
}

void AFarmTaskPlayerController::TryFire()
{
	if (Character)
	{
		Character->OnFire();
	}
}

void AFarmTaskPlayerController::TryInteract()
{
	if (Character)
	{
		Character->CallInteraction();
	}
}

void AFarmTaskPlayerController::TryJump()
{
	if (Character)
	{
		Character->Jump();
	}
}

void AFarmTaskPlayerController::TryStopJumping()
{
	if (Character)
	{
		Character->StopJumping();
	}
}

void AFarmTaskPlayerController::TryMoveForward(const float Val)
{
	if (Character)
	{
		Character->MoveForward(Val);
	}
}

void AFarmTaskPlayerController::TryMoveRight(const float Val)
{
	if (Character)
	{
		Character->MoveRight(Val);
	}
}

void AFarmTaskPlayerController::TryTurnAtRate(const float Rate)
{
	if (Character)
	{
		Character->TurnAtRate(Rate, BaseTurnRate);
	}
}

void AFarmTaskPlayerController::TryLookUpAtRate(const float Rate)
{
	if (Character)
	{
		Character->LookUpAtRate(Rate, BaseLookUpRate);
	}
}

void AFarmTaskPlayerController::BeginPlay()
{
	Super::BeginPlay();

	Character = Cast<AFarmTaskCharacter>(UGameplayStatics::GetPlayerCharacter(this, NetPlayerIndex));
}
