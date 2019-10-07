// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthBar.h"


void UHealthBar::InitializeHealthValues(const float MaxHealthVal, float HealthVal)
{
	MaxHealth = MaxHealthVal;
	Health = HealthVal;
}

void UHealthBar::OnDamageTaken()
{
	if (Health > 0)
	{
		--Health;
	}
}
