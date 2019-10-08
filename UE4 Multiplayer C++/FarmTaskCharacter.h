// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InteractionComponent.h"
#include "FarmTaskCharacter.generated.h"

class UInputComponent;

UCLASS(config=Game)
class AFarmTaskCharacter : public ACharacter
{
	GENERATED_BODY()

#pragma region Private Variables

private:

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Replicated, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Replicated, Category = Mesh)
	class USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly,  Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

#pragma endregion

#pragma region Public Variables

public:

	//Interaction component
	UPROPERTY(BlueprintReadWrite, Category = Interaction)
		UInteractionComponent* InteractionComponent;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Replicated, Category = Projectile)
		TSubclassOf<class AFarmTaskProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = Gameplay)
		class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = Gameplay)
		class UAnimMontage* FireAnimation;

#pragma endregion

#pragma region Private Methods

private:

	AFarmTaskCharacter();

	virtual void BeginPlay();

#pragma endregion

#pragma region Public Methods

public:

	// Shoot
	UFUNCTION(Server, Reliable)
	void OnFire();

	UFUNCTION(Server, Reliable)
	void ServerCallInteraction();

	UFUNCTION(NetMulticast, Reliable)
	void PlayFireSoundAndAnimation();

	void CallInteraction();

	void MoveForward(const float Val);

	void MoveRight(const float Val);

	void TurnAtRate(const float Rate, const float BaseTurnRate);

	void LookUpAtRate(const float Rate, const float BaseLookUpRate);

	void Tick(float DeltaTime) override;

	//Get camera view rotation
	FRotator GetViewRotation() const override;

	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

#pragma endregion

};

