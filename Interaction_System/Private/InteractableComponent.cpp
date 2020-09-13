// Copyright Andrzej Serazetdinow, 2020 All Rights Reserved.

#include "InteractableComponent.h"
#include "NameWidget.h"
#include "InteractionWidgetOnInteractable.h"
#include "InteractionLog.h"

#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"

#include "Components/WidgetComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"

#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#include "Net/UnrealNetwork.h"

UInteractableComponent::UInteractableComponent()
	: bCanBroadcastCanInteract(true), InteractionWidgetOnInteractableUsable(false), InteractionMarkerUsable(false),
	NameWidgetUsable(false), CanShowInteractionMarker(true)
{
	PrimaryComponentTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(FName("InteractionCollision"));
	InteractionMarker = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractableMarker"));
	InteractionWidgetOnInteractable = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionWidgetOnInteractable"));
	InteractableName = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractableNameComponent"));

	SetIsReplicatedByDefault(true);

	if (GetOwner())
	{
		SphereComponent->AttachToComponent(GetOwner()->GetRootComponent(),
			FAttachmentTransformRules::KeepRelativeTransform);
		this->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

		InteractionMarker->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
		InteractionWidgetOnInteractable->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
		InteractableName->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	}
}

void UInteractableComponent::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (OtherActor && OtherActor != GetOwner() && OtherComp &&
		Cast<APawn>(OtherActor) && Cast<APawn>(OtherActor)->IsLocallyControlled())
	{
		if (UPlayerInteractionComponent* PIC = OtherActor->
			FindComponentByClass<UPlayerInteractionComponent>()
		)
		{
			PIC->AddActorToInteract(GetOwner());
		}
	}
}

void UInteractableComponent::OnOverlapEnd(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
)
{
	if (OtherActor && OtherActor != GetOwner() && OtherComp &&
		Cast<APawn>(OtherActor) && Cast<APawn>(OtherActor)->IsLocallyControlled())
	{
		if (UPlayerInteractionComponent* PIC = OtherActor->
			FindComponentByClass<UPlayerInteractionComponent>()
		)
		{
			PIC->RemoveActorToInteract(GetOwner());
		}
	}
}

void UInteractableComponent::Interact(UPlayerInteractionComponent* PIC)
{
	if (!PIC)
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("UPlayerInteractionComponent passed to Interact() is nullptr."));
		return;
	}

	if (InteractDelegate.IsBound())
	{
		InteractDelegate.Broadcast(PIC->GetOwner());
	}
}

void UInteractableComponent::UnsubscribeFromComponent(AActor* Player)
{
	if (!Player)
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("Player passed to UnsubscribeFromComponent() is nullptr."));
		return;
	}

	if (!SubscribedPlayers.Contains(Player))
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("Player passed to UnsubscribeFromComponent() is not cached inside subscribed players."));
		return;
	}

	SubscribedPlayers.Remove(Player);

	AmountOfSubscribedPlayers--;

	if (OnUnsubscribedDelegate.IsBound())
	{
		OnUnsubscribedDelegate.Broadcast(Player);
	}

	if (SubscribedPlayers.Num() <= 0)
	{
		SetComponentTickEnabled(false);
	}

	InteractionWidgetOnInteractableUsable = false;
	InteractionMarkerUsable = false;
	NameWidgetUsable = false;
}

void UInteractableComponent::SubscribeToComponent(AActor* Player, bool CurrentlySelected)
{
	if (!Player)
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("Player passed to SubscribeToComponent() is nullptr."));
		return;
	}

	if(SubscribedPlayers.Contains(Player))
	{
		return;
	}

	SubscribedPlayers.Add(Player);
	++AmountOfSubscribedPlayers;

	UPlayerInteractionComponent* Temp = SubscribedPlayers[AmountOfSubscribedPlayers - 1]
		->FindComponentByClass<UPlayerInteractionComponent>();

	if (!Temp)
	{
		SubscribedPlayers.Remove(Player);
		--AmountOfSubscribedPlayers;

		UE_LOG(InteractionSystem, Warning,
			TEXT("Tried to subscribe to a player %s with invalid interaction component or without one in SubscribeToComponent()."), *GetNameSafe(Player));
		return;
	}

	if (CurrentlySelected && Temp->CanSelectOnlyOneInteractable)
	{
		if (OnSelectedDelegate.IsBound())
		{
			OnSelectedDelegate.Broadcast(Player);
		}
	}

	SetComponentTickEnabled(true);

	PlayerComponents.Add(Temp);

	if (OnSubscribedDelegate.IsBound())
	{
		OnSubscribedDelegate.Broadcast(Player);
	}
}

bool UInteractableComponent::IsSubscribed(const UPlayerInteractionComponent* PlayerComponent) const
{
	if (!PlayerComponent)
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("Tried to check if PlayerComponent is subscribed but PlayerComponent passed to IsSubscribed() is nullptr."));
		return false;
	}

	return PlayerComponents.Contains(PlayerComponent);
}

bool UInteractableComponent::CanInteract(const AActor* Player)
{
	if (!Player)
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("Tried to check if player can interact but Player passed to CanInteract() is nullptr."));
		return false;
	}

	if (InteractableStructure.bDisabled)
	{
		return false;
	}

	if (InteractableStructure.bDoesDistanceToPlayerMatter)
	{
		if (CheckDistanceToPlayer(Player) > InteractableStructure.MaximumDistanceToPlayer)
		{
			return false;
		}
	}

	if (InteractableStructure.bHasToBeReacheable)
	{
		if (!CheckReachability(Player))
		{
			return false;
		}
	}

	if (InteractableStructure.bDoesAngleMatter)
	{
		UPlayerInteractionComponent* PlayerInteractionComponent = Cast<UPlayerInteractionComponent>(
			Player->FindComponentByClass(UPlayerInteractionComponent::StaticClass()));

		if (!PlayerInteractionComponent)
		{
			UE_LOG(InteractionSystem, Warning,
				TEXT("Tried to check angle to player but PlayerInteractionComponent in CanInteract() is nullptr."));
			return false;
		}

		if (CheckAngleToPlayer(Player) == FAILED_Angle ? false : PlayerInteractionComponent->
			bPlayerHasToLookOnTheObjectInsteadOfCheckingAngle && PlayerInteractionComponent->bIsUsingFirstPersonMode ?
			CheckAngleToPlayer(Player) != PlayerLooksAtInteractableValue :
			CheckAngleToPlayer(Player) > InteractableStructure.PlayersAngleMarginOfErrorToInteractable)
		{
			return false;
		}
	}

	return true;
}

bool UInteractableComponent::CheckReachability(const AActor* SubscribedPlayer) const
{
	if (!SubscribedPlayer)
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("Tried to check reachability but SubscribedPlayer in CheckReachability() is nullptr."));
		return false;
	}

	if (GetOwner() && SubscribedPlayers.Num() > 0 && GetWorld())
	{
		const FVector& InteractableLocation = GetComponentLocation();
		const FVector& PlayerLocation = SubscribedPlayer->GetActorLocation();

		FCollisionQueryParams CollisionParams;
		FHitResult OutHit;

		CollisionParams.AddIgnoredActor(GetOwner());

		if (InteractableStructure.bDrawDebugLineForReachability)
		{
			DrawDebugLine(GetWorld(), PlayerLocation, InteractableLocation, FColor::Green,
				false, 0.1f, 1, 1.f);
		}

		do 
		{
			if (GetWorld()->LineTraceSingleByChannel(OutHit, InteractableLocation, PlayerLocation,
				ECC_Visibility, CollisionParams))
			{
				if (OutHit.bBlockingHit)
				{
					if (OutHit.GetActor() && OutHit.GetActor()->
						FindComponentByClass<UPlayerInteractionComponent>())
					{
						return true;
					}
					else if (OutHit.Component.IsValid() && (OutHit.Component->IsA<USphereComponent>()
						|| OutHit.Component->IsA<UWidgetComponent>()))
					{
						CollisionParams.AddIgnoredComponent(OutHit.Component.Get());
					}
					else
					{
						return false;
					}
				}
			}
		} while (OutHit.Actor.IsValid() && (OutHit.Actor.Get()->FindComponentByClass<UInteractableComponent>()));
	}

	return false;
}

float UInteractableComponent::CheckDistanceToPlayer(const AActor* SubscribedPlayer) const
{
	if (!SubscribedPlayer)
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("Tried to check Check Distance To Player but SubscribedPlayer in CheckDistanceToPlayer() is nullptr."));
		return -1.f;
	}

	if (GetOwner() && SubscribedPlayers.Num() > 0)
	{
		const FVector& InteractableLocation = GetComponentLocation();
		const FVector& PlayerLocation = SubscribedPlayer->GetActorLocation();

		return FVector::Dist(InteractableLocation, PlayerLocation);
	}

	return -1.f;
}

float UInteractableComponent::CheckAngleToPlayer(const AActor* SubscribedPlayer) const
{
	FVector InteractableLocation = GetComponentLocation();

	if (!SubscribedPlayer)
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("Tried to check Check Angle To Player but SubscribedPlayer in CheckAngleToPlayer() is nullptr."));
		return FAILED_Angle;
	}

	UPlayerInteractionComponent* PlayerInteractionComponent = SubscribedPlayer
		->FindComponentByClass<UPlayerInteractionComponent>();

	if (!PlayerInteractionComponent)
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("Tried to check Check Angle To Player but PlayerInteractionComponent in CheckAngleToPlayer() is nullptr."));
		return FAILED_Angle;
	}

	if (PlayerInteractionComponent->bIsUsingFirstPersonMode)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

		if (!PlayerController)
		{
			UE_LOG(InteractionSystem, Warning,
				TEXT("Tried to check Check Angle To Player but PlayerController in CheckAngleToPlayer() is nullptr."));
			return FAILED_Angle;
		}

		if (PlayerInteractionComponent->bPlayerHasToLookOnTheObjectInsteadOfCheckingAngle)
		{
			FHitResult HitResult(ForceInit);

			FCollisionQueryParams CamTraceParams = FCollisionQueryParams(FName(
				TEXT("InteractionTrace")), true);
			CamTraceParams.bTraceComplex = true;
			CamTraceParams.bReturnPhysicalMaterial = true;
			CamTraceParams.AddIgnoredActor(SubscribedPlayer);

			APlayerCameraManager* PCM = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);

			if (!PCM)
			{
				UE_LOG(InteractionSystem, Warning,
					TEXT("Tried to check Check Angle To Player but PlayerCameraManager in CheckAngleToPlayer() is nullptr."));
				return FAILED_Angle;
			}

			FVector TraceStart = PCM->GetCameraLocation();
			FVector TraceEnd = TraceStart + PCM->GetCameraRotation().Vector() * 1000000.f;

			GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Camera,
				CamTraceParams);

			if (HitResult.bBlockingHit && HitResult.GetActor() && HitResult.GetActor() == GetOwner())
			{
				return PlayerLooksAtInteractableValue;
			}

		}
		else
		{
			FVector2D ComponentScreenLocation;
			FVector2D PlayerScreenCenter;

			UGameplayStatics::ProjectWorldToScreen(PlayerController, GetComponentLocation(),
				ComponentScreenLocation);

			int32 viewportX;
			int32 viewportY;

			PlayerController->GetViewportSize(viewportX, viewportY);

			PlayerScreenCenter = { viewportX * 0.5f, viewportY * 0.5f };

			PlayerScreenCenter.Normalize();
			ComponentScreenLocation.Normalize();

			float Dot = FVector2D::DotProduct(ComponentScreenLocation, PlayerScreenCenter);

			return FMath::Acos(Dot) * Multiplier; //Rad to Degrees
		}
	}
	else
	{
		UArrowComponent* Arrow = SubscribedPlayer
			->FindComponentByClass<UArrowComponent>();

		if (!Arrow)
		{
			UE_LOG(InteractionSystem, Warning,
				TEXT("Tried to check Check Angle To Player but Arrow in CheckAngleToPlayer() is nullptr."));
			return FAILED_Angle;
		}

		if (Arrow)
		{
			FVector PlayerForwardVector = Arrow->GetForwardVector();
			FVector const PlayerToInteractableLocation = (InteractableLocation - SubscribedPlayer->
				GetActorLocation()).GetSafeNormal();

			PlayerForwardVector.Normalize();

			float Dot = FVector::DotProduct(PlayerForwardVector, PlayerToInteractableLocation);

			return FMath::Acos(Dot) * Multiplier; //Rad to Degrees
		}
	}

	return 0.f;
}

void UInteractableComponent::DrawDebugStrings(const AActor* Player) const
{
	if (!Player)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Player passed to DrawDebugStrings() is nullptr."));
		return;
	}

	UPlayerInteractionComponent* PlayerInteractionComponent = Player
		->FindComponentByClass<UPlayerInteractionComponent>();

	if (!PlayerInteractionComponent)
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("Player interaction component in DrawDebugStrings() is nullptr."));
		return;
	}

	if (GetWorld() && SubscribedPlayers.Num() > 0)
	{
		uint8 AmountOfDebugStrings = 1;

		// Priority
		DrawDebugString(GetWorld(),
			InteractableStructure.bOverrideDebugStringLocation ? InteractableStructure.NewDebugStringLocation
			: PlayerInteractionComponent->DSProperties.bUseOwningActorLocationForDebugText ? GetOwner()->GetActorLocation() : GetComponentLocation(), FString("Priority: ") + FString::FromInt(InteractableStructure.Priority), NULL,
			InstancedDSP.ValidTextColor, 0.01f, InstancedDSP.bDrawShadow/*, InstancedDSP.FontScale*/);

		// Whether or not the object is disabled
		DrawDebugString(GetWorld(),
			InteractableStructure.bOverrideDebugStringLocation ? InteractableStructure.NewDebugStringLocation
			: PlayerInteractionComponent->DSProperties.bUseOwningActorLocationForDebugText ? FVector(GetOwner()->GetActorLocation().X, GetOwner()->GetActorLocation().Y, GetOwner()->GetActorLocation().Z - InstancedDSP.HeightDifferenceInDebugStrings)
			: FVector(GetComponentLocation().X, GetComponentLocation().Y, GetComponentLocation().Z - InstancedDSP.HeightDifferenceInDebugStrings),
			FString("Usability: ") + (InteractableStructure.bDisabled ? FString("Disabled") : FString("Enabled")), NULL,
			!InteractableStructure.bDisabled ? InstancedDSP.ValidTextColor : InstancedDSP.InvalidTextColor,
			0.01f, InstancedDSP.bDrawShadow/*, InstancedDSP.FontScale*/);

		AmountOfDebugStrings++;

		// Distance to player
		if (InteractableStructure.bDoesDistanceToPlayerMatter)
		{
			DrawDebugString(GetWorld(),
				InteractableStructure.bOverrideDebugStringLocation ? InteractableStructure.NewDebugStringLocation
				: PlayerInteractionComponent->DSProperties.bUseOwningActorLocationForDebugText ? FVector(GetOwner()->GetActorLocation().X, GetOwner()->GetActorLocation().Y, GetOwner()->GetActorLocation().Z - (AmountOfDebugStrings * InstancedDSP.HeightDifferenceInDebugStrings))
				: FVector(GetComponentLocation().X, GetComponentLocation().Y, GetComponentLocation().Z - (AmountOfDebugStrings * InstancedDSP.HeightDifferenceInDebugStrings)),
				FString("Distance: ") + FString::SanitizeFloat(CheckDistanceToPlayer(Player), 2), NULL,
				CheckDistanceToPlayer(Player) < InteractableStructure.MaximumDistanceToPlayer ? InstancedDSP.ValidTextColor : InstancedDSP.InvalidTextColor,
				0.01f, InstancedDSP.bDrawShadow/*, InstancedDSP.FontScale*/);

			AmountOfDebugStrings++;
		}

		// Reachability
		if (InteractableStructure.bHasToBeReacheable)
		{
			DrawDebugString(GetWorld(),
				InteractableStructure.bOverrideDebugStringLocation ? InteractableStructure.NewDebugStringLocation
				: PlayerInteractionComponent->DSProperties.bUseOwningActorLocationForDebugText ? FVector(GetOwner()->GetActorLocation().X, GetOwner()->GetActorLocation().Y, GetOwner()->GetActorLocation().Z - (AmountOfDebugStrings * InstancedDSP.HeightDifferenceInDebugStrings))
				: FVector(GetComponentLocation().X, GetComponentLocation().Y, GetComponentLocation().Z - (AmountOfDebugStrings * InstancedDSP.HeightDifferenceInDebugStrings)),
				CheckReachability(Player) ? FString("Reachability: Reachable") : FString("Reachability: Not Reachable"), NULL,
				CheckReachability(Player) ? InstancedDSP.ValidTextColor : InstancedDSP.InvalidTextColor,
				0.01f, InstancedDSP.bDrawShadow/*, InstancedDSP.FontScale*/);

			AmountOfDebugStrings++;
		}

		// Angle
		if (InteractableStructure.bDoesAngleMatter)
		{
			DrawDebugString(GetWorld(),
				InteractableStructure.bOverrideDebugStringLocation ? InteractableStructure.NewDebugStringLocation
				: PlayerInteractionComponent->DSProperties.bUseOwningActorLocationForDebugText ? FVector(GetOwner()->GetActorLocation().X, GetOwner()->GetActorLocation().Y, GetOwner()->GetActorLocation().Z - (AmountOfDebugStrings * InstancedDSP.HeightDifferenceInDebugStrings))
				: FVector(GetComponentLocation().X, GetComponentLocation().Y, GetComponentLocation().Z - (AmountOfDebugStrings * InstancedDSP.HeightDifferenceInDebugStrings)),
				CheckAngleToPlayer(Player) == FAILED_Angle ? FString("Failed calculating angle.") : PlayerInteractionComponent->bPlayerHasToLookOnTheObjectInsteadOfCheckingAngle && PlayerInteractionComponent->bIsUsingFirstPersonMode ? CheckAngleToPlayer(Player) == PlayerLooksAtInteractableValue ? FString("Player Looks At Interactable") : FString("Player Is Not Looking At Interactable") : FString("Angle: ") + FString::SanitizeFloat(CheckAngleToPlayer(Player), 2), NULL,
				CheckAngleToPlayer(Player) == FAILED_Angle ? InstancedDSP.InvalidTextColor : PlayerInteractionComponent->bPlayerHasToLookOnTheObjectInsteadOfCheckingAngle && PlayerInteractionComponent->bIsUsingFirstPersonMode ? CheckAngleToPlayer(Player) == PlayerLooksAtInteractableValue ? InstancedDSP.ValidTextColor : InstancedDSP.InvalidTextColor : CheckAngleToPlayer(Player) <= InteractableStructure.PlayersAngleMarginOfErrorToInteractable ? InstancedDSP.ValidTextColor : InstancedDSP.InvalidTextColor,
				0.01f, InstancedDSP.bDrawShadow/*, InstancedDSP.FontScale*/);

			AmountOfDebugStrings++;
		}
	}
}

void UInteractableComponent::HideInteractionWidgetOnInteractable()
{
	if (InteractionWidgetOnInteractable && InteractionWidgetOnInteractable->IsVisible())
	{
		InteractionWidgetOnInteractable->SetVisibility(false);
	}
}

void UInteractableComponent::HideInteractionMarker()
{
	if (InteractionMarker && InteractionMarker->IsVisible())
	{
		InteractionMarker->SetVisibility(false);
	}
}

void UInteractableComponent::ShowInteractionWidgetOnInteractable(UInteractionWidgetOnInteractable* Widget)
{
	if (!Widget)
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("Widget passed to ShowInteractionWidgetOnInteractable() is nullptr."));
		return;
	}

	if (!InteractionWidgetOnInteractable)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("UWidgetComponent InteractionWidgetOnInteractable in ShowInteractionWidgetOnInteractable() is nullptr."));
		return;
	}

	Widget->OnTextChanged(InteractableStructure.InteractionText);
	InteractionWidgetOnInteractable->SetWidget(Widget);

	if (!InteractionWidgetOnInteractable->IsVisible())
	{
		InteractionWidgetOnInteractable->SetVisibility(true);
	}

	if (OnInteractionWidgetOnInteractableUsable.IsBound() 
		&& !InteractionWidgetOnInteractableUsable)
	{
		InteractionWidgetOnInteractableUsable = true;
		OnInteractionWidgetOnInteractableUsable.Broadcast();
	}

	if (InteractionWidgetOnInteractableClass != Widget->GetClass())
	{
		InteractionWidgetOnInteractableClass = Widget->GetClass();
	}
}

void UInteractableComponent::ShowInteractionMarker(UUserWidget* Widget)
{
	if (!Widget)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Widget passed to ShowInteractionMarker() is nullptr."));
		return;
	}

	if (!InteractionMarker)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("UWidgetComponent InteractionMarker in ShowInteractionMarker() is nullptr."));
		return;
	}

	InteractionMarker->SetWidget(Widget);

	if (!InteractionMarker->IsVisible())
	{
		InteractionMarker->SetVisibility(true);
	}

	if (OnInteractionMarkerUsable.IsBound() && !InteractionMarkerUsable)
	{
		InteractionMarkerUsable = true;
		OnInteractionMarkerUsable.Broadcast();
	}

	if (InteractableMarkerClass != Widget->GetClass())
	{
		InteractableMarkerClass = Widget->GetClass();
	}
}

void UInteractableComponent::Enable()
{
	InteractableStructure.bDisabled = false;
}

void UInteractableComponent::Disable()
{
	InteractableStructure.bDisabled = true;
}

void UInteractableComponent::BeginPlay()
{
	InteractionMarker->SetVisibility(false);
	InteractionWidgetOnInteractable->SetVisibility(false);
	InteractableName->SetVisibility(false);

	if (InteractableStructure.bRandomizePriority)
	{
		InteractableStructure.Priority = FMath::RandRange(InteractableStructure.PriorityRandomizedMIN,
			InteractableStructure.PriorityRandomizedMAX);
	}

	if (RandomizeRarityValue)
	{
		RarityValue = FMath::RandRange(RarityRandomizedMIN, RarityRandomizedMAX);
	}

	SetComponentTickEnabled(true);

	GetWorld()->GetTimerManager().SetTimer(InteractionTimerHandle, this, &UInteractableComponent::CheckOverlappingActors, 1.f, false, 0.2f);

	SphereComponent->OnComponentBeginOverlap.AddDynamic(this,
		&UInteractableComponent::OnOverlapBegin);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this,
		&UInteractableComponent::OnOverlapEnd);

	Super::BeginPlay();
}

void UInteractableComponent::CheckOverlappingActors()
{
	TArray<AActor*> OverlappingActors;
	SphereComponent->GetOverlappingActors(OverlappingActors, TSubclassOf<AActor>());

	for (const auto& Actor : OverlappingActors)
	{
		if (Actor != GetOwner())
		{
			if (UPlayerInteractionComponent* Component = Actor->
				FindComponentByClass<UPlayerInteractionComponent>())
			{
				Component->AddActorToInteract(GetOwner());
			}
		}
	}
}

bool UInteractableComponent::IsAnySubscribedPlayerLocallyControlled()
{
	if (GetLocallyControlledPlayer())
	{
		return true;
	}

	return false;
}

APawn* UInteractableComponent::GetLocallyControlledPlayer() const
{
	for (const auto& SubscribedPlayer : SubscribedPlayers)
	{
		APawn* PlayerPawn = Cast<APawn>(SubscribedPlayer);

		if (PlayerPawn && PlayerPawn->IsLocallyControlled())
		{
			return PlayerPawn;
		}
	}

	return nullptr;
}

void UInteractableComponent::SetWidgetRotationSettings(bool IsCameraRotation, bool IsPawnRotation)
{
	if (IsCameraRotation)
	{
		bRotateWidgetsTowardsCamera = true;
		bRotateWidgetsTowardsPlayerPawnCMP = false;
	}
	else if (IsPawnRotation)
	{
		bRotateWidgetsTowardsCamera = false;
		bRotateWidgetsTowardsPlayerPawnCMP = true;
	}
	else
	{
		bRotateWidgetsTowardsCamera = false;
		bRotateWidgetsTowardsPlayerPawnCMP = false;
	}
}

void UInteractableComponent::RotateWidgetsToPlayer(bool ToCamera)
{
	if (!GetLocallyControlledPlayer())
	{
		UE_LOG(InteractionSystem, Error, TEXT("ERROR: Trying to rotate widgets in RotateWidgetsToPlayer() without local player."));
		return;
	}

	WidgetRotation = UKismetMathLibrary::FindLookAtRotation(InteractionWidgetOnInteractable->GetComponentLocation(),
		ToCamera ? UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation()
		: GetLocallyControlledPlayer()->GetActorLocation());

	if (InteractionWidgetOnInteractable && InteractionWidgetOnInteractable->IsVisible())
	{
		InteractionWidgetOnInteractable->SetWorldRotation(WidgetRotation);
	}

	if (InteractionMarker && InteractionMarker->IsVisible())
	{
		InteractionMarker->SetWorldRotation(WidgetRotation);
	}

	if (InteractableName && InteractableName->IsVisible())
	{
		InteractableName->SetWorldRotation(WidgetRotation);
	}
}

void UInteractableComponent::BroadcastCanInteract(const UPlayerInteractionComponent* PlayerComponent) const
{
	if (!PlayerComponent)
	{
		UE_LOG(InteractionSystem, Warning,
			TEXT("PlayerComponent passed to BroadcastCanInteract() is nullptr."));
		return;
	}

	if (PlayerComponent->OnCanInteractDelegate.IsBound())
	{
		PlayerComponent->OnCanInteractDelegate.Broadcast(GetOwner());
	}

	if (OnCanInteractDelegate.IsBound())
	{
		OnCanInteractDelegate.Broadcast(PlayerComponent->GetOwner());
	}
}

bool UInteractableComponent::CanAnyPlayerInteract()
{
	for (const auto& Component : PlayerComponents)
	{
		if (Component.IsValid() && Component.Get() &&
			CanInteract(Component.Get()->GetOwner()))
		{
			return true;
		}
	}

	return false;
}

void UInteractableComponent::HideInteractableName()
{
	if (InteractableName && InteractableName->IsVisible())
	{
		InteractableName->SetVisibility(false);
	}
}

void UInteractableComponent::ShowInteractableName(UNameWidget* Widget)
{
	if (!Widget)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Widget passed to ShowInteractableName() is nullptr."));
		return;
	}

	Widget->OnNameChanged(InteractableStructure.InteractableName);

	if (!InteractableName)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("UWidgetComponent InteractableName in ShowInteractableName() is nullptr."));
		return;
	}

	InteractableName->SetWidget(Widget);

	if (!InteractableName->IsVisible())
	{
		InteractableName->SetVisibility(true);
	}

	if (OnNameWidgetUsable.IsBound() && !NameWidgetUsable)
	{
		NameWidgetUsable = true;
		OnNameWidgetUsable.Broadcast();
	}

	if (InteractableNameClass != Widget->GetClass())
	{
		InteractableNameClass = Widget->GetClass();
	}
}

void UInteractableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInteractableComponent, InteractableStructure);
}

void UInteractableComponent::TryHideWidgets(UPlayerInteractionComponent* PlayerComponent)
{
	if (!PlayerComponent)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("PlayerComponent passed to TryHideWidgets() is nullptr."));
		return;
	}

	if (((PlayerComponent->bShowOnlyOneInteractableName
		&& PlayerComponent->InteractableInteracted.IsValid()
		&& PlayerComponent->InteractableInteracted.Get() != this)
		|| InteractableStructure.bDisabled)
		|| (PlayerComponent->bHideInteractableNameWhenInteractableIsUnreachable
		&&	!CheckReachability(PlayerComponent->GetOwner()))
		)
	{
		PlayerComponent->TryHideInteractableName(this);
	}
	else
	{
		PlayerComponent->TryShowInteractableName(this);
	}
	PlayerComponent->TryHideInteractionWidget(this);
	PlayerComponent->TryHideInteractionWidgetOnInteractable(this);
	PlayerComponent->TryHideInteractionProgress(this);
}

void UInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (SubscribedPlayers.Num())
	{
		if ((InstancedDSP.bDrawDebugStringsByDefault && InteractableStructure.bAlwaysDrawDebugStrings)
			|| InteractableStructure.bAlwaysDrawDebugStrings)
		{
			if (GetLocallyControlledPlayer())
			{
#if !UE_BUILD_SHIPPING

				DrawDebugStrings(GetLocallyControlledPlayer());

#endif //!UE_BUILD_SHIPPING
			}
			else
			{
				SetComponentTickEnabled(false);
				return;
			}
		}
	}

	if (PlayerComponents.Num())
	{
		for (const auto& Component : PlayerComponents)
		{
			if (!InteractableStructure.bDisabled && CheckReachability(Component->GetOwner()))
			{
				if (!InteractionMarker->IsVisible())
				{
					Component->TryShowInteractionMarker(this);
				}

				if (Component->CanSelectOnlyOneInteractable &&
					!Component->InteractableInteracted.IsValid() &&
					CanInteract(Component->GetOwner())
				)
				{
					Component->InteractableInteracted = this;
				}
				else if (Component->CanSelectOnlyOneInteractable 
					&& Component->InteractableInteracted.IsValid() 
					&& CanInteract(Component->GetOwner())
					&& Component->InteractableInteracted.Get() != this
				)
				{
					if (CanInteract(Component->GetOwner()))
					{
						Component->InteractableInteracted = this;
					}
				}

				if (!Component->InteractableInteracted.IsValid() ||
					Component->InteractableInteracted.Get() != this)
				{
					TryHideWidgets(Component.Get());
				}

				if (!InteractableName->IsVisible() 
					&& !Component->bShowOnlyOneInteractableName
					|| !Component->InteractableInteracted.IsValid())
				{
					if (!Component->InteractableInteracted.IsValid())
					{
						Component->InteractableInteracted = this;
					}

					Component->TryShowInteractableName(this);
				}

				if (!InteractableName->IsVisible() &&
					Component->bShowOnlyOneInteractableName &&
					Component->InteractableInteracted.IsValid() &&
					Component->InteractableInteracted.Get() == this)
				{
					Component->TryShowInteractableName(this);
				}
				else if (InteractableName->IsVisible() &&
					Component->bShowOnlyOneInteractableName &&
					Component->InteractableInteracted.IsValid() &&
					Component->InteractableInteracted.Get() != this)
				{
					Component->TryHideInteractableName(this);
				}

				if (CanInteract(Component->GetOwner()))
				{
					Component->TryShowInteractionWidget(this);
					Component->TryShowInteractionWidgetOnInteractable(this);
					
					if (bCanBroadcastCanInteract)
					{
						BroadcastCanInteract(Component.Get());
						bCanBroadcastCanInteract = false;
					}
				}
				else
				{
					if (!CanAnyPlayerInteract())
					{
						bCanBroadcastCanInteract = true;
					}

					Component->TryHideInteractionWidget(this);
					Component->TryHideInteractionWidgetOnInteractable(this);
				}
			}
			else
			{
				if (Component->bHideInteractionMarkerWhenInteractableIsUnreachable)
				{
					Component->TryHideInteractionMarker(this);
				}
				else if (!InteractionMarker->IsVisible())
				{
					Component->TryShowInteractionMarker(this);
				}

				TryHideWidgets(Component.Get());
			}
		}
	}

	if (InteractionMarker->IsVisible() || InteractionWidgetOnInteractable->IsVisible())
	{
		if (bUseRotationVariablesFromPlayerComponent)
		{
			if (bRotateWidgetsTowardsCamera)
			{
				RotateWidgetsToPlayer(true);
			}
			else if (bRotateWidgetsTowardsPlayerPawnCMP)
			{
				RotateWidgetsToPlayer(false);
			}
		}
		else if (bRotateWidgetsTowardsPlayerCamera)
		{
			RotateWidgetsToPlayer(true);
		}
		else if (bRotateWidgetsTowardsPlayerPawn)
		{
			RotateWidgetsToPlayer(false);
		}
	}
}

