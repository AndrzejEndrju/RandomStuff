// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "FarmTaskProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "DestructibleComponent.h"
#include "Components/SphereComponent.h"

AFarmTaskProjectile::AFarmTaskProjectile() 
{
	bReplicates = true;
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AFarmTaskProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	CollisionComp->SetSimulatePhysics(false);
	CollisionComp->SetNotifyRigidBodyCollision(true);

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 30 seconds by default
	InitialLifeSpan = 30.0f;
}

void AFarmTaskProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL))
	{
		FPointDamageEvent DamageEvent;
		FHitResult HitResult;
		HitResult.ImpactPoint = GetActorLocation();
		HitResult.ImpactNormal = GetActorLocation().GetSafeNormal();
		DamageEvent.HitInfo = HitResult;

		if (UDestructibleComponent* DC = OtherActor->FindComponentByClass<UDestructibleComponent>())
		{
			DC->ApplyDamage(500.f, HitResult.ImpactPoint, HitResult.ImpactNormal, 100.f);
			return;
		}

		if (OtherComp->IsSimulatingPhysics())
		{
			OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
		}
	}
}