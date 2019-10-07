// Fill out your copyright notice in the Description page of Project Settings.


#include "DestructibleProp.h"
#include "FarmTaskProjectile.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "Components/WidgetComponent.h"
#include "DestructibleComponent.h"
#include "Engine.h"
//#include "HealthBar.h"
#include "Kismet/KismetMathLibrary.h"
#include "UnrealNetwork.h"

ADestructibleProp::ADestructibleProp()
{
	bReplicates = true;

	IsDestroyed = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");
	Widget = CreateDefaultSubobject<UWidgetComponent>("Widget");
	Destructible = CreateDefaultSubobject<UDestructibleComponent>(TEXT("Destructible"));

	Destructible->SetupAttachment(RootComponent);
	Destructible->SetNotifyRigidBodyCollision(true);

	MaxHealth = 1.f;

	DefaultDamage = 50000.f;
	DefaultImpulse = 50000.f;
}

void ADestructibleProp::BeginPlay()
{
	Super::BeginPlay();

	Destructible->OnComponentHit.AddDynamic(this, &ADestructibleProp::Damage);

	CurrentHealth = MaxHealth;

	/*if (Widget)
	{
		WidgetHealthBar = Cast<UHealthBar>(Widget->GetUserWidgetObject());
	}*/

	/*if (WidgetHealthBar)
	{
		WidgetHealthBar->InitializeHealthValues(MaxHealth, CurrentHealth);
	}*/
}

void ADestructibleProp::Damage(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!IsDestroyed && OtherActor == Cast<AFarmTaskProjectile>(OtherActor))
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, "Hit By: " + OtherActor->GetName());

		CurrentHealth = CurrentHealth - 0.1f;
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, "Current Health: " + FString::SanitizeFloat(CurrentHealth));
		Destructible->ApplyDamage(DefaultDamage, Hit.Location, NormalImpulse, DefaultImpulse);

		/*if (WidgetHealthBar)
		{
			WidgetHealthBar->OnDamageTaken();
		}*/
		
		if (CurrentHealth <= 0.f)
		{
			IsDestroyed = true;
			OnExplode();
		}
	}
}

void ADestructibleProp::OnRep_Destroy(bool OldState)
{
	if (IsDestroyed && !OldState)
	{
		OnExplode();
	}
}

void ADestructibleProp::OnExplode()
{
	Destructible->AddRadialImpulse(GetActorLocation(), 1000.f, 1000.f, ERadialImpulseFalloff::RIF_Constant, false);
	Destructible->AddRadialForce(GetActorLocation(), 1000.f, 1000.f, ERadialImpulseFalloff::RIF_Constant, false);

	if (Particle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Particle, FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 15), FRotator(0.f, 0.f, 0.f));
	}

	if (Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, GetActorLocation(), 1.f, 1.f, 0.f);
	}

	if (Widget)
	{
		Widget->DestroyComponent();
	}
}

void ADestructibleProp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADestructibleProp, IsDestroyed);
	DOREPLIFETIME(ADestructibleProp, CurrentHealth);
	DOREPLIFETIME(ADestructibleProp, Destructible);
}

void ADestructibleProp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	WidgetRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetActorLocation());
	
	/*if (Widget)
	{
		Widget->SetWorldRotation(FRotator(0.f, WidgetRotation.Yaw, WidgetRotation.Roll));
	}*/
}

