#include "GameCameraEventer.h"

#include "Components/SplineComponent.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"

#if WITH_EDITOR

#include "Misc/MessageDialog.h"
#include "UObject/Package.h"

#endif

AGameCameraEventer::AGameCameraEventer()
{
	PrimaryActorTick.bCanEverTick = true;

	OnActorBeginOverlap.AddDynamic(this, &AGameCameraEventer::OnActorEntered);
	OnActorEndOverlap.AddDynamic(this, &AGameCameraEventer::OnActorLeft);
}

void AGameCameraEventer::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<AGameCamera> It(GetWorld()); It; ++It)
	{
		GameCamera = *It;
		break;
	}

	CheckExistingOverlap();
}

void AGameCameraEventer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PendingActor.IsValid())
	{
		if (!PendingActor->IsOverlappingActor(this))
		{
			PendingActor = nullptr;
			ActivationTimer = 0.f;
		}
		else if ((!bFireOnlyOnce || !bTriggered) && (!GameCamera || !GameCamera->IsLastTriggeredEventer(this)))
		{
			if (ActivationDelay <= 0.f)
			{
				ActivateEventer(PendingActor.Get());
				PendingActor = nullptr;
				ActivationTimer = 0.f;
			}
			else
			{
				ActivationTimer += DeltaTime;

				if (ActivationTimer >= ActivationDelay)
				{
					ActivateEventer(PendingActor.Get());
					PendingActor = nullptr;
					ActivationTimer = 0.f;
				}
			}
		}
	}

	if (!PendingActor.IsValid() && (!bFireOnlyOnce || !bTriggered))
	{
		CheckExistingOverlap();
	}

	if (bPathActive && GameCamera && GameCamera->IsLastTriggeredEventer(this))
	{
		UpdatePath(DeltaTime);
	}
}

void AGameCameraEventer::OnActorEntered(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	if (!OtherActor->IsA<ACharacter>())
	{
		return;
	}

	if (bFireOnlyOnce && bTriggered)
	{
		return;
	}

	if (GameCamera && GameCamera->IsLastTriggeredEventer(this))
	{
		return;
	}

	PendingActor = OtherActor;
	ActivationTimer = 0.f;

	if (ActivationDelay <= 0.f)
	{
		ActivateEventer(OtherActor);
		PendingActor = nullptr;
	}
}

void AGameCameraEventer::ActivateEventer(AActor* OtherActor)
{
	if (!OtherActor)
	{
		return;
	}

	if (bFireOnlyOnce && bTriggered)
	{
		return;
	}

	if (GameCamera && GameCamera->IsLastTriggeredEventer(this))
	{
		return;
	}

	GameCamera = nullptr;

	for (TActorIterator<AGameCamera> It(GetWorld()); It; ++It)
	{
		if (It->GetOwner() == OtherActor)
		{
			GameCamera = *It;
			break;
		}
	}

	if (!GameCamera)
	{
		UE_LOG(LogTemp, Error, TEXT("EVENTER: Camera for %s not found"), *GetNameSafe(OtherActor));
		return;
	}

	if (GameCamera->IsLastTriggeredEventer(this))
	{
		return;
	}

	bTriggered = true;

	GameCamera->SetLastTriggeredEventer(this);

	ApplyCameraSettings();

	if (CameraPathActor)
	{
		StartPath();
	}
}

void AGameCameraEventer::OnActorLeft(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	if (!OtherActor->IsA<ACharacter>())
	{
		return;
	}

	if (PendingActor.Get() == OtherActor)
	{
		PendingActor = nullptr;
		ActivationTimer = 0.f;
	}

	if (!GameCamera)
	{
		return;
	}

	const bool bWasLastTriggeredEventer = GameCamera->IsLastTriggeredEventer(this);

	StopPath();

	if (bWasLastTriggeredEventer)
	{
		GameCamera->ClearLastTriggeredEventer(this);
	}

	if (!bRestoreOnExit)
	{
		return;
	}

	if (RestoreCameraPathActor)
	{
		CameraPathActor = RestoreCameraPathActor;
		StartDistanceThreshold = RestoreStartDistanceThreshold;
		PathOffset = RestorePathOffset;
		FollowSmoothing = RestoreFollowSmoothing;
		bPathLoop = bRestorePathLoop;

		if (bWasLastTriggeredEventer)
		{
			GameCamera->SetLastTriggeredEventer(this);
			StartPath();
		}
	}
}

void AGameCameraEventer::CheckExistingOverlap()
{
	TArray<AActor*> OverlappingActors;

	GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (!Actor || !Actor->IsA<ACharacter>())
		{
			continue;
		}

		if (bFireOnlyOnce && bTriggered)
		{
			return;
		}

		if (PendingActor.Get() == Actor)
		{
			continue;
		}

		if (GameCamera && GameCamera->IsLastTriggeredEventer(this))
		{
			continue;
		}

		OnActorEntered(this, Actor);
		return;
	}
}

void AGameCameraEventer::ApplyTargets(AActor* NewConnectTarget)
{
	if (!GameCamera)
	{
		return;
	}

	GameCamera->SetConnectTarget(NewConnectTarget);
}

void AGameCameraEventer::ApplyCameraSettings()
{
	if (!GameCamera)
	{
		return;
	}

	ApplyTargets(ConnectTarget);

	GameCamera->SetCameraState(CameraState, TransitionMode);
}

void AGameCameraEventer::StartPath()
{
	if (!GameCamera || !CameraPathActor)
	{
		return;
	}

	USplineComponent* Spline = CameraPathActor->FindComponentByClass<USplineComponent>();

	if (!Spline)
	{
		return;
	}

	ActiveSpline = Spline;

	ActiveStartDistanceThreshold = StartDistanceThreshold;
	ActivePathOffset = PathOffset;
	ActiveFollowSmoothing = FollowSmoothing;
	bActivePathLoop = bPathLoop;

	float ClosestDistance = 0.f;

	if (!FindClosestPathDistance(ClosestDistance))
	{
		ClosestDistance = 0.f;
	}

	CurrentPathDistance = FMath::Clamp(ClosestDistance, 0.f, ActiveSpline->GetSplineLength());

	bPathActive = true;
	bPathFinished = false;

	ApplyPathRotation();
}

void AGameCameraEventer::StopPath()
{
	const bool bIsLastTriggeredEventer = GameCamera && GameCamera->IsLastTriggeredEventer(this);

	bPathActive = false;
	bPathFinished = false;
	ActiveSpline = nullptr;

	if (!GameCamera || !bIsLastTriggeredEventer)
	{
		return;
	}

	GameCamera->ClearPathRotation();

	if (!bRestoreOnExit)
	{
		return;
	}

	GameCamera->SetConnectTarget(RestoreConnectTarget);
	GameCamera->SetCameraState(RestoreCameraState, RestoreTransitionMode);
}

bool AGameCameraEventer::FindClosestPathDistance(float& OutDistance) const
{
	OutDistance = 0.f;

	if (!ActiveSpline || !GameCamera)
	{
		return false;
	}

	AActor* Target = GameCamera->GetConnectTarget();

	if (!Target)
	{
		Target = GameCamera->GetPlayerTarget();
	}

	if (!Target)
	{
		return false;
	}

	const FVector TargetLocation = Target->GetActorLocation();
	const float InputKey = ActiveSpline->FindInputKeyClosestToWorldLocation(TargetLocation);

	OutDistance = ActiveSpline->GetDistanceAlongSplineAtSplineInputKey(InputKey);

	return true;
}

void AGameCameraEventer::UpdatePath(float DeltaTime)
{
	if (!bPathActive || !GameCamera || !ActiveSpline)
	{
		return;
	}

	if (!GameCamera->IsLastTriggeredEventer(this))
	{
		return;
	}

	AActor* Target = GameCamera->GetConnectTarget();

	if (!Target)
	{
		Target = GameCamera->GetPlayerTarget();
	}

	if (!Target)
	{
		return;
	}

	const float SplineLength = ActiveSpline->GetSplineLength();

	if (SplineLength <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const FVector TargetWorldLocation = Target->GetActorLocation();

	const float ClosestKey = ActiveSpline->FindInputKeyClosestToWorldLocation(TargetWorldLocation);

	const float PlayerDistance = ActiveSpline->GetDistanceAlongSplineAtSplineInputKey(ClosestKey);

	float TargetDistance = PlayerDistance;

	if (PlayerDistance > ActiveStartDistanceThreshold)
	{
		TargetDistance = (PlayerDistance - ActiveStartDistanceThreshold) + ActivePathOffset;
	}
	else
	{
		TargetDistance = ActivePathOffset;
	}

	if (bActivePathLoop)
	{
		TargetDistance = FMath::Fmod(TargetDistance, SplineLength);

		if (TargetDistance < 0.f)
		{
			TargetDistance += SplineLength;
		}
	}
	else
	{
		TargetDistance = FMath::Clamp(TargetDistance, 0.f, SplineLength);
	}

	if (TransitionMode == ECameraTransitionMode::Instant)
	{
		CurrentPathDistance = TargetDistance;
	}
	else
	{
		CurrentPathDistance = FMath::FInterpTo(CurrentPathDistance, TargetDistance, DeltaTime, ActiveFollowSmoothing);
	}

	const FVector WorldPosition = ActiveSpline->GetLocationAtDistanceAlongSpline(CurrentPathDistance, ESplineCoordinateSpace::World);

	FGameCameraState State = CameraState;

	switch (CameraState.PositionMode)
	{
	case ECameraPositionMode::World:
	{
		State.Position = WorldPosition;
		break;
	}

	case ECameraPositionMode::PlayerTarget:
	{
		AActor* PlayerTarget = GameCamera->GetPlayerTarget();

		if (PlayerTarget)
		{
			State.Position = PlayerTarget->GetActorTransform().InverseTransformPosition(WorldPosition);
		}
		else
		{
			State.Position = WorldPosition;
		}

		break;
	}

	case ECameraPositionMode::ConnectTarget:
	{
		AActor* TargetActor = GameCamera->GetConnectTarget();

		if (TargetActor)
		{
			State.Position = TargetActor->GetActorTransform().InverseTransformPosition(WorldPosition);
		}
		else
		{
			State.Position = WorldPosition;
		}

		break;
	}

	default:
	{
		State.Position = WorldPosition;
		break;
	}
	}

	if (CameraState.bUsePathRotation)
	{
		const FRotator PathRotation = ActiveSpline->GetRotationAtDistanceAlongSpline(CurrentPathDistance, ESplineCoordinateSpace::World);

		GameCamera->SetPathRotation(PathRotation);
	}
	else
	{
		GameCamera->ClearPathRotation();
	}

	GameCamera->SetCameraState(State, TransitionMode);

	if (!bActivePathLoop)
	{
		if (CurrentPathDistance >= SplineLength - 1.f)
		{
			bPathFinished = true;
		}
		else if (CurrentPathDistance <= 1.f)
		{
			bPathFinished = true;
		}
		else
		{
			bPathFinished = false;
		}
	}
}

void AGameCameraEventer::ApplyPathRotation()
{
	if (!GameCamera || !ActiveSpline)
	{
		return;
	}

	if (!GameCamera->IsLastTriggeredEventer(this))
	{
		return;
	}

	if (!CameraState.bUsePathRotation)
	{
		GameCamera->ClearPathRotation();
		return;
	}

	const FRotator WorldRotation = ActiveSpline->GetRotationAtDistanceAlongSpline(CurrentPathDistance, ESplineCoordinateSpace::World);

	if (CameraState.PositionMode == ECameraPositionMode::World)
	{
		GameCamera->SetPathRotation(WorldRotation);
		return;
	}

	AActor* Target = nullptr;

	if (CameraState.PositionMode == ECameraPositionMode::PlayerTarget)
	{
		Target = GameCamera->GetPlayerTarget();
	}
	else if (CameraState.PositionMode == ECameraPositionMode::ConnectTarget)
	{
		Target = GameCamera->GetConnectTarget();
	}

	if (!Target)
	{
		GameCamera->SetPathRotation(WorldRotation);
		return;
	}

	const FRotator RelativeRotation = Target->GetActorTransform().InverseTransformRotation(WorldRotation.Quaternion()).Rotator();

	GameCamera->SetPathRotation(RelativeRotation);
}

#if WITH_EDITOR

bool AGameCameraEventer::ConfirmOverwrite() const
{
	const EAppReturnType::Type Result = FMessageDialog::Open(
		EAppMsgType::YesNo,
		NSLOCTEXT(
			"GameCameraEventer",
			"OverwritePreset",
			"Preset already exists. Overwrite it?"
		)
	);

	return Result == EAppReturnType::Yes;
}

AActor* AGameCameraEventer::FindActorByName(const FName& ActorName) const
{
	if (ActorName.IsNone() || !GetWorld())
	{
		return nullptr;
	}

	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->GetFName() == ActorName)
		{
			return *It;
		}
	}

	return nullptr;
}

void AGameCameraEventer::SavePreset()
{
	if (!PresetData || PresetName.IsNone())
	{
		return;
	}

	FGameCameraEventerPreset NewPreset;
	NewPreset.PresetName = PresetName;
	NewPreset.CameraState = CameraState;

	PresetData->Presets.Add(NewPreset);

	PresetData->MarkPackageDirty();
}

void AGameCameraEventer::LoadPreset()
{
	if (!PresetData || PresetName.IsNone())
	{
		return;
	}

	for (const FGameCameraEventerPreset& Preset : PresetData->Presets)
	{
		if (Preset.PresetName == PresetName)
		{
			CameraState = Preset.CameraState;
			return;
		}
	}
}

void AGameCameraEventer::DeletePreset()
{
	if (!PresetData || PresetName.IsNone())
	{
		return;
	}

	for (int32 i = PresetData->Presets.Num() - 1; i >= 0; --i)
	{
		if (PresetData->Presets[i].PresetName == PresetName)
		{
			PresetData->Presets.RemoveAt(i);
			PresetData->MarkPackageDirty();
			return;
		}
	}
}

#endif