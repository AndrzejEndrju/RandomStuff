// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "FarmTaskGameMode.h"
#include "FarmTaskHUD.h"
#include "FarmTaskCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFarmTaskGameMode::AFarmTaskGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
	// set default pc
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/MyFarmTaskPlayerController"));
	PlayerControllerClass = PlayerControllerClassFinder.Class;
	// set default HUD
	static ConstructorHelpers::FClassFinder<AHUD> HUDClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/MyFarmTaskHUD"));
	HUDClass = HUDClassFinder.Class;
}
