// Fill out your copyright notice in the Description page of Project Settings.


#include "Lamp.h"
#include "Components/PointLightComponent.h"
#include "UnrealNetwork.h"
#include "FarmTaskCharacter.h"

ALamp::ALamp()
{
	bReplicates = true;

	UStaticMeshComponent* mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cube Mesh"));

	IsLightOn = true;

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetLightBrightness(50.f);
	Light->SetLightColor(FLinearColor(1.f, 0.031194f, 0.041141f, 1.f));
	Light->SourceLength = 15.f;
	Light->Intensity = 10000.f;
	Light->AttenuationRadius = 250.f;
}

void ALamp::OnRep_IsLightOn()
{
	if (!Light)
	{
		return;
	}

	if (IsLightOn)
	{
		Light->SetLightBrightness(50.f);
		Light->Intensity = 10000.f;
		Light->SetLightColor(FLinearColor(1.f, 0.031194f, 0.041141f, 1.f));
	}
	else
	{
		Light->Intensity = 0.f;
		Light->SetLightBrightness(0.f);
		Light->SetLightColor(FLinearColor(1.f, 1.f, 1.f, 1.f));
	}
}

void ALamp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALamp, Light);
	DOREPLIFETIME(ALamp, IsLightOn);
}

bool ALamp::Interact()
{
	IsLightOn = !IsLightOn;

	if (Light)
	{
		OnRep_IsLightOn();
		return true;
	}
	
	return false;
}


