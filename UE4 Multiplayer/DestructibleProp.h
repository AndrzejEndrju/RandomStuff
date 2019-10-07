// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DestructibleProp.generated.h"

class UDestructibleComponent;
class UParticleSystem;
class UWidgetComponent;
//class UHealthBar;
class USoundCue;

UCLASS()
class FARMTASK_API ADestructibleProp : public AActor
{
	GENERATED_BODY()
	
#pragma region Public Variables

public:

	/*UHealthBar* WidgetHealthBar;*/

	UPROPERTY(EditAnywhere, Replicated, Category = Destructible)
	UDestructibleComponent* Destructible;

	UPROPERTY(EditAnywhere, Category = Destructible)
	UWidgetComponent* Widget;

	UPROPERTY(EditAnywhere, Replicated, Category = Destructible)
	UParticleSystem* Particle;

	UPROPERTY(EditAnywhere, Replicated, Category = Destructible)
	USoundCue* Sound;

	UPROPERTY(EditInstanceOnly, ReplicatedUsing = OnRep_Destroy, BlueprintReadOnly, Category = Destructible)
	bool IsDestroyed;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = Destructible)
	float MaxHealth;

	UPROPERTY(EditInstanceOnly, Replicated, BlueprintReadOnly, Category = Destructible)
	float CurrentHealth;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = Destructible)
	float DefaultDamage;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = Destructible)
	float DefaultImpulse;

#pragma endregion

#pragma region Private Variables

	FRotator WidgetRotation;

#pragma endregion

#pragma region Public Methods

public:	
	
	//Deal damage to destructible object
	UFUNCTION()
	void Damage(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	//Replication method check
	UFUNCTION()
	void OnRep_Destroy(bool OldState);

	//Explode the destructible, play particle effect, sound effect and destroy widget component
	void OnExplode();

#pragma endregion

#pragma region Private Methods

private:

	ADestructibleProp();

	virtual void BeginPlay() override;

	void Tick(float DeltaTime) override;

#pragma endregion
};
