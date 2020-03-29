// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "Interaction_System.h"

#define LOCTEXT_NAMESPACE "FInteraction_SystemModule"

void FInteraction_SystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FInteraction_SystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FInteraction_SystemModule, Interaction_System)