// Made by Andrzej Serazetdinow

#include "InteractableComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"

#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/WidgetComponent.h"

#include "UnrealNetwork.h"

#include "InteractionLog.h"

UInteractableComponent::UInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	InteractionMarker = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractableMarker"));
	InteractionWidgetOnInteractable = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionWidgetOnInteractable"));

	bCanBroadcastCanInteract = true;

	bReplicates = true;

	CanShowInteractionMarker = true;
	IsInteractionMarkerHidden = true;
	IsInteractionWidgetOnInteractableHidden = true;

	if (GetOwner())
	{
		this->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

		InteractionMarker->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
		InteractionWidgetOnInteractable->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	}
}

void UInteractableComponent::Interact()
{
	if (InteractDelegate.IsBound())
	{
		InteractDelegate.Broadcast();
	}
}

void UInteractableComponent::UnsubscribeFromComponent(AActor* Player)
{
	if (!Player)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Player passed to UnsubscribeFromComponent() is nullptr."));
		return;
	}

	if (!SubscribedPlayers.Contains(Player))
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Player passed to UnsubscribeFromComponent() is not cached inside subscribed players."));
		return;
	}

	SubscribedPlayers.Remove(Player);

	UPlayerInteractionComponent* PIC = Cast<UPlayerInteractionComponent>(Player->FindComponentByClass(UPlayerInteractionComponent::StaticClass()));

	if (PIC)
	{
		PlayerComponents.Remove(PIC);
	}

	AmountOfSubscribedPlayers--;

	if (OnUnsubscribedDelegate.IsBound())
	{
		OnUnsubscribedDelegate.Broadcast();
	}

	if (SubscribedPlayers.Num() <= 0)
	{
		SetComponentTickEnabled(false);
	}
}

void UInteractableComponent::SubscribeToComponent(AActor* Player)
{
	if (!Player)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Player passed to SubscribeToComponent() is nullptr."));
		return;
	}

	SubscribedPlayers.Add(Player);
	AmountOfSubscribedPlayers++;

	UPlayerInteractionComponent* Temp = Cast<UPlayerInteractionComponent>(SubscribedPlayers[AmountOfSubscribedPlayers - 1]->FindComponentByClass(UPlayerInteractionComponent::StaticClass()));

	if (!Temp)
	{
		SubscribedPlayers.Remove(Player);
		AmountOfSubscribedPlayers--;

		UE_LOG(InteractionSystem, Warning, TEXT("Tried to subscribe to a player %s with invalid interaction component or without one in SubscribeToComponent()."), *GetNameSafe(Player));
		return;
	}

	SetComponentTickEnabled(true);
	PlayerComponents.Add(Temp);

	if (OnSubscribedDelegate.IsBound())
	{
		OnSubscribedDelegate.Broadcast();
	}
}

const uint8 UInteractableComponent::GetPriority()
{
	return InteractableStructure.Priority;
}

const bool UInteractableComponent::CanInteract(AActor* Player)
{
	if (!Player)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Tried to check if player can interact but Player passed to CanInteract() is nullptr."));
		return false;
	}

	if (InteractableStructure.bDisabled)
	{
		return false;
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
		UPlayerInteractionComponent* PlayerInteractionComponent = Cast<UPlayerInteractionComponent>(Player->FindComponentByClass(UPlayerInteractionComponent::StaticClass()));

		if (!PlayerInteractionComponent)
		{
			UE_LOG(InteractionSystem, Warning, TEXT("Tried to check angle to player but PlayerInteractionComponent in CanInteract() is nullptr."));
			return false;
		}

		if (CheckAngleToPlayer(Player) == FAILED_Angle ? false : PlayerInteractionComponent->bPlayerHasToLookOnTheObjectInsteadOfCheckingAngle && PlayerInteractionComponent->bIsUsingFirstPersonMode ?
			CheckAngleToPlayer(Player) != PlayerLooksAtInteractableValue : CheckAngleToPlayer(Player) > InteractableStructure.PlayersAngleMarginOfErrorToInteractable)
		{
			return false;
		}
	}

	if (InteractableStructure.bDoesDistanceToPlayerMatter)
	{
		if (CheckDistanceToPlayer(Player) > InteractableStructure.MaximumDistanceToPlayer)
		{
			return false;
		}
	}

	return true;
}

bool UInteractableComponent::CheckReachability(AActor* SubscribedPlayer) const
{
	if (!SubscribedPlayer)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Tried to check reachability but SubscribedPlayer in CheckReachability() is nullptr."));
		return false;
	}

	if (GetOwner() && SubscribedPlayers.Num() > 0 && GetWorld())
	{
		const FVector& InteractableLocation = GetComponentLocation();
		const FVector& PlayerLocation = SubscribedPlayer->GetActorLocation();

		FCollisionQueryParams CollisionParams;
		FHitResult OutHit;

		if (InteractableStructure.bDrawDebugLineForReachability)
		{
			DrawDebugLine(GetWorld(), PlayerLocation, InteractableLocation, FColor::Green, false, 0.1f, 1, 1.f);
		}

		if (GetWorld()->LineTraceSingleByChannel(OutHit, PlayerLocation, InteractableLocation, ECC_Visibility, CollisionParams))
		{
			if (OutHit.bBlockingHit)
			{
				if (OutHit.GetActor() && OutHit.GetActor()->FindComponentByClass(this->GetClass()) == this)
				{
					return true;
				}
			}
		}
	}

	return false;
}

const float UInteractableComponent::CheckDistanceToPlayer(AActor* SubscribedPlayer) const
{
	if (!SubscribedPlayer)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Tried to check Check Distance To Player but SubscribedPlayer in CheckDistanceToPlayer() is nullptr."));
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

const float UInteractableComponent::CheckAngleToPlayer(AActor* SubscribedPlayer) const
{
	FVector InteractableLocation = GetComponentLocation();

	if (!SubscribedPlayer)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Tried to check Check Angle To Player but SubscribedPlayer in CheckAngleToPlayer() is nullptr."));
		return FAILED_Angle;
	}

	UPlayerInteractionComponent* PlayerInteractionComponent = Cast<UPlayerInteractionComponent>(SubscribedPlayer->FindComponentByClass(UPlayerInteractionComponent::StaticClass()));

	if (!PlayerInteractionComponent)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Tried to check Check Angle To Player but PlayerInteractionComponent in CheckAngleToPlayer() is nullptr."));
		return FAILED_Angle;
	}

	if (PlayerInteractionComponent->bIsUsingFirstPersonMode)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

		if (!PlayerController)
		{
			UE_LOG(InteractionSystem, Warning, TEXT("Tried to check Check Angle To Player but PlayerController in CheckAngleToPlayer() is nullptr."));
			return FAILED_Angle;
		}

		if (PlayerInteractionComponent->bPlayerHasToLookOnTheObjectInsteadOfCheckingAngle)
		{
			FHitResult HitResult(ForceInit);

			FCollisionQueryParams CamTraceParams = FCollisionQueryParams(FName(TEXT("InteractionTrace")), true);
			CamTraceParams.bTraceComplex = true;
			CamTraceParams.bReturnPhysicalMaterial = true;
			CamTraceParams.AddIgnoredActor(SubscribedPlayer);

			APlayerCameraManager* PCM = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);

			if (!PCM)
			{
				UE_LOG(InteractionSystem, Warning, TEXT("Tried to check Check Angle To Player but PlayerCameraManager in CheckAngleToPlayer() is nullptr."));
				return FAILED_Angle;
			}

			FVector TraceStart = PCM->GetCameraLocation();
			FVector TraceEnd = TraceStart + PCM->GetCameraRotation().Vector() * 1000000.f;

			GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Camera, CamTraceParams);

			if (HitResult.bBlockingHit && HitResult.GetActor() && HitResult.GetActor() == GetOwner())
			{
				return PlayerLooksAtInteractableValue;
			}

		}
		else
		{
			FVector2D ComponentScreenLocation;
			FVector2D PlayerScreenCenter;

			UGameplayStatics::ProjectWorldToScreen(PlayerController, GetComponentLocation(), ComponentScreenLocation);

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
		UArrowComponent* Arrow = Cast<UArrowComponent>(SubscribedPlayer->FindComponentByClass(UArrowComponent::StaticClass()));

		if (!Arrow)
		{
			UE_LOG(InteractionSystem, Warning, TEXT("Tried to check Check Angle To Player but Arrow in CheckAngleToPlayer() is nullptr."));
			return FAILED_Angle;
		}

		if (Arrow)
		{
			FVector PlayerForwardVector = Arrow->GetForwardVector();
			FVector const PlayerToInteractableLocation = (InteractableLocation - SubscribedPlayer->GetActorLocation()).GetSafeNormal();

			PlayerForwardVector.Normalize();

			float Dot = FVector::DotProduct(PlayerForwardVector, PlayerToInteractableLocation);

			return FMath::Acos(Dot) * Multiplier; //Rad to Degrees
		}
	}

	return 0.f;
}

void UInteractableComponent::DrawDebugStrings(AActor* Player)
{
	if (!Player)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Player passed to DrawDebugStrings() is nullptr."));
		return;
	}

	UPlayerInteractionComponent* PlayerInteractionComponent = Cast<UPlayerInteractionComponent>(Player->FindComponentByClass(UPlayerInteractionComponent::StaticClass()));

	if (!PlayerInteractionComponent)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Player interaction component in DrawDebugStrings() is nullptr."));
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
				CheckAngleToPlayer(Player) == FAILED_Angle ? InstancedDSP.InvalidTextColor  : PlayerInteractionComponent->bPlayerHasToLookOnTheObjectInsteadOfCheckingAngle && PlayerInteractionComponent->bIsUsingFirstPersonMode ? CheckAngleToPlayer(Player) == PlayerLooksAtInteractableValue ? InstancedDSP.ValidTextColor : InstancedDSP.InvalidTextColor : CheckAngleToPlayer(Player) <= InteractableStructure.PlayersAngleMarginOfErrorToInteractable ? InstancedDSP.ValidTextColor : InstancedDSP.InvalidTextColor,
				0.01f, InstancedDSP.bDrawShadow/*, InstancedDSP.FontScale*/);

			AmountOfDebugStrings++;
		}
	}
}

void UInteractableComponent::HideInteractionWidgetOnInteractable()
{
	if (InteractionWidgetOnInteractable)
	{
		InteractionWidgetOnInteractable->SetVisibility(false);
		IsInteractionWidgetOnInteractableHidden = true;
	}
}

void UInteractableComponent::HideInteractionMarker()
{
	if (InteractionMarker)
	{
		InteractionMarker->SetVisibility(false);
		IsInteractionMarkerHidden = true;
	}
}

void UInteractableComponent::ShowInteractionWidgetOnInteractable(UUserWidget* Widget)
{
	if (!Widget)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Widget passed to ShowInteractionWidgetOnInteractable() is nullptr."));
		return;
	}

	if (InteractionWidgetOnInteractable->GetWidgetClass() != Widget->GetClass())
	{
		InteractionWidgetOnInteractable->SetWidget(Widget);
		InteractionWidgetOnInteractable->SetVisibility(true);

		if (OnInteractionWidgetOnInteractableUsable.IsBound())
		{
			OnInteractionWidgetOnInteractableUsable.Broadcast();
		}
	}

	else
	{
		InteractionWidgetOnInteractable->SetVisibility(true);
		IsInteractionWidgetOnInteractableHidden = false;
	}
}

void UInteractableComponent::ShowInteractionMarker(UUserWidget* Widget)
{
	if (!Widget)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("Widget passed to ShowInteractionMarker() is nullptr."));
		return;
	}

	if (InteractionMarker->GetWidgetClass() != Widget->GetClass())
	{
		InteractionMarker->SetWidget(Widget);
		InteractionMarker->SetVisibility(true);

		if (OnInteractionMarkerUsable.IsBound())
		{
			OnInteractionMarkerUsable.Broadcast();
		}
	}

	else
	{
		InteractionMarker->SetVisibility(true);
		IsInteractionMarkerHidden = false;
	}
}

void UInteractableComponent::Enable()
{
	if (InteractableStructure.bDisabled)
	{
		InteractableStructure.bDisabled = false;
	}
}

void UInteractableComponent::Disable(bool bDisableForEveryone)
{
	if (!InteractableStructure.bDisabled)
	{
		if (InteractableStructure.bChangeEnabilityOnlyLocally)
		{
			if(!bDisableForEveryone && IsAnySubscribedPlayerLocallyControlled())
			{ }
		}
		InteractableStructure.bDisabled = true;
	}
}

void UInteractableComponent::BeginPlay()
{
	if (InteractableStructure.bRandomizePriority)
	{
		InteractableStructure.Priority = FMath::RandRange(InteractableStructure.PriorityRandomizedMIN, InteractableStructure.PriorityRandomizedMAX);
	}

	if (RandomizeRarityValue)
	{
		RarityValue = FMath::RandRange(RarityRandomizedMIN, RarityRandomizedMAX);
	}

	SetComponentTickEnabled(false);

	Super::BeginPlay();
}

bool UInteractableComponent::IsAnySubscribedPlayerLocallyControlled()
{
	if (GetLocallyControlledPlayer())
	{
		return true;
	}

	return false;
}

APawn* UInteractableComponent::GetLocallyControlledPlayer()
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

void UInteractableComponent::RotateWidgetsToPlayerCamera()
{
	WidgetRotation = UKismetMathLibrary::FindLookAtRotation(GetOwner()->GetActorLocation(), UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation());

	if (InteractionWidgetOnInteractable->IsVisible())
	{
		InteractionWidgetOnInteractable->SetWorldRotation(WidgetRotation);
	}

	if (InteractionMarker->IsVisible())
	{
		InteractionMarker->SetWorldRotation(WidgetRotation);
	}
}

void UInteractableComponent::RotateWidgetsToPlayerPawn()
{
	WidgetRotation = UKismetMathLibrary::FindLookAtRotation(GetOwner()->GetActorLocation(), GetLocallyControlledPlayer()->GetActorLocation());

	if (InteractionWidgetOnInteractable->IsVisible())
	{
		InteractionWidgetOnInteractable->SetWorldRotation(WidgetRotation);
	}

	if (InteractionMarker->IsVisible())
	{
		InteractionMarker->SetWorldRotation(WidgetRotation);
	}
}

void UInteractableComponent::BroadcastCanInteract(UPlayerInteractionComponent* PlayerComponent)
{
	if (!PlayerComponent)
	{
		UE_LOG(InteractionSystem, Warning, TEXT("PlayerComponent passed to BroadcastCanInteract() is nullptr."));
		return;
	}

	if (PlayerComponent->OnCanInteractDelegate.IsBound())
	{
		PlayerComponent->OnCanInteractDelegate.Broadcast();
	}

	if (OnCanInteractDelegate.IsBound())
	{
		OnCanInteractDelegate.Broadcast();
	}
}

bool UInteractableComponent::CanAnyPlayerInteract()
{
	for (const auto& Component : PlayerComponents)
	{
		if (CanInteract(Component->GetOwner()))
		{
			return true;
		}
	}

	return false;
}

void UInteractableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInteractableComponent, InteractableStructure);
}

void UInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (SubscribedPlayers.Num() > 0)
	{
		if ((InstancedDSP.bDrawDebugStringsByDefault && InteractableStructure.bAlwaysDrawDebugStrings) || InteractableStructure.bAlwaysDrawDebugStrings)
		{
			if(GetLocallyControlledPlayer())
			{
#if !UE_BUILD_SHIPPING

				DrawDebugStrings(GetLocallyControlledPlayer());

#endif //!UE_BUILD_SHIPPING
			}
			else
			{
				SetComponentTickEnabled(false);
			}
		}
	}
	
	if (PlayerComponents.Num() > 0)
	{
		for (const auto& Component : PlayerComponents)
		{
			if (!InteractableStructure.bDisabled)
			{
				if (IsInteractionMarkerHidden)
				{
					Component->TryShowInteractionMarker(this);
				}

				if (CanInteract(Component->GetOwner()))
				{
					Component->TryShowInteractionWidget(this);
					Component->TryShowInteractionWidgetOnInteractable(this);

					if (bCanBroadcastCanInteract)
					{
						BroadcastCanInteract(Component);
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
				Component->TryHideInteractionWidget(this);
				Component->TryHideInteractionWidgetOnInteractable(this);
			}
		}
	}

	if (InteractionMarker->IsVisible() || InteractionWidgetOnInteractable->IsVisible())
	{
		if (bUseRotationVariablesFromPlayerComponent)
		{
			if (bRotateWidgetsTowardsCamera)
			{
				RotateWidgetsToPlayerCamera();
			}
			else if (bRotateWidgetsTowardsPlayerPawnCMP)
			{
				RotateWidgetsToPlayerPawn();
			}
		}
		else if (bRotateWidgetsTowardsPlayerCamera)
		{
			RotateWidgetsToPlayerCamera();
		}
		else if (bRotateWidgetsTowardsPlayerPawn)
		{
			RotateWidgetsToPlayerPawn();
		}
	}
		
}

