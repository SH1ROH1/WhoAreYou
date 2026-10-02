#include "GameCameraController.h"

#include "EngineUtils.h"
#include "GameCamera.h"
#include "Components/SplineComponent.h"
#include "GameCameraEventer.h"
#include "Kismet/GameplayStatics.h"

AGameCameraController::AGameCameraController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGameCameraController::BeginPlay()
{
	Super::BeginPlay();

	FindAllCameras();
}

void AGameCameraController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckActivationDelay(DeltaTime);

	if (bPathActive)
	{
		UpdatePath(DeltaTime);
	}
}


//==========================================================
// Camera Search
//==========================================================

void AGameCameraController::FindAllCameras()
{
	CachedCameras.Empty();

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	for (TActorIterator<AGameCamera> It(World); It; ++It)
	{
		if (AGameCamera* Camera = *It)
		{
			CachedCameras.Add(Camera);
		}
	}
}

AGameCamera* AGameCameraController::FindCameraByTag(FName Tag)
{
	if (Tag.IsNone())
	{
		return nullptr;
	}

	if (CachedCameras.Num() == 0)
	{
		FindAllCameras();
	}

	for (AGameCamera* Camera : CachedCameras)
	{
		if (!IsValid(Camera))
		{
			continue;
		}

		if (Camera->ActorHasTag(Tag))
		{
			return Camera;
		}
	}

	return nullptr;
}

AGameCamera* AGameCameraController::ResolveTargetCamera() const
{
	if (bUseCameraTag && !CameraTag.IsNone())
	{
		for (AGameCamera* Camera : CachedCameras)
		{
			if (!IsValid(Camera))
			{
				continue;
			}

			if (Camera->ActorHasTag(CameraTag))
			{
				return Camera;
			}
		}
	}

	if (IsValid(GameCamera))
	{
		return GameCamera;
	}

	return nullptr;
}

AGameCamera* AGameCameraController::GetTargetCamera()
{
	AGameCamera* Camera = ResolveTargetCamera();

	if (Camera)
	{
		return Camera;
	}

	FindAllCameras();

	return ResolveTargetCamera();
}


//==========================================================
// Apply
//==========================================================

void AGameCameraController::ApplyToCamera(
	AGameCamera* TargetCamera,
	const FGameCameraState& State,
	AActor* NewConnectTarget,
	ECameraTransitionMode NewTransitionMode
)
{
	if (!IsValid(TargetCamera))
	{
		return;
	}

	TargetCamera->SetConnectTarget(NewConnectTarget);
	TargetCamera->SetCameraState(State, NewTransitionMode);
}


//==========================================================
// Main Command
//==========================================================

void AGameCameraController::ApplyCameraCommand()
{
	if (bAffectAllCameras)
	{
		if (CachedCameras.Num() == 0)
		{
			FindAllCameras();
		}

		for (AGameCamera* Camera : CachedCameras)
		{
			if (!IsValid(Camera))
			{
				continue;
			}

			ApplyToCamera(
				Camera,
				CameraState,
				ConnectTarget,
				TransitionMode
			);
		}

		return;
	}

	AGameCamera* TargetCamera = GetTargetCamera();

	if (!TargetCamera)
	{
		return;
	}

	ApplyToCamera(
		TargetCamera,
		CameraState,
		ConnectTarget,
		TransitionMode
	);
}

void AGameCameraController::ApplyCameraCommandToCamera(
	AGameCamera* TargetCamera
)
{
	if (!IsValid(TargetCamera))
	{
		return;
	}

	ApplyToCamera(
		TargetCamera,
		CameraState,
		ConnectTarget,
		TransitionMode
	);
}


//==========================================================
// Camera State
//==========================================================

void AGameCameraController::ApplyCameraState(
	const FGameCameraState& NewState
)
{
	CameraState = NewState;

	ApplyCameraCommand();
}

void AGameCameraController::SetCameraState(
	const FGameCameraState& NewState
)
{
	CameraState = NewState;
}

FGameCameraState AGameCameraController::GetCameraState() const
{
	return CameraState;
}

void AGameCameraController::GetCameraStateFromCamera(
	AGameCamera* TargetCamera,
	FGameCameraState& OutState
) const
{
	OutState = FGameCameraState();

	if (!IsValid(TargetCamera))
	{
		return;
	}

	OutState = TargetCamera->GetState();
}


//==========================================================
// Position
//==========================================================

void AGameCameraController::SetCameraPosition(
	FVector NewPosition
)
{
	CameraState.Position = NewPosition;

	ApplyCameraCommand();
}

void AGameCameraController::SetCameraPositionMode(
	ECameraPositionMode NewMode
)
{
	CameraState.PositionMode = NewMode;

	ApplyCameraCommand();
}

void AGameCameraController::SetCameraTransform(
	FVector NewPosition,
	FRotator NewRotation,
	ECameraPositionMode NewPositionMode
)
{
	CameraState.Position = NewPosition;
	CameraState.Rotation = NewRotation;
	CameraState.PositionMode = NewPositionMode;

	ApplyCameraCommand();
}


//==========================================================
// Rotation
//==========================================================

void AGameCameraController::SetCameraRotation(
	FRotator NewRotation
)
{
	CameraState.Rotation = NewRotation;

	ApplyCameraCommand();
}

void AGameCameraController::SetPathRotation(
	FRotator NewRotation
)
{
	if (bAffectAllCameras)
	{
		if (CachedCameras.Num() == 0)
		{
			FindAllCameras();
		}

		for (AGameCamera* Camera : CachedCameras)
		{
			if (IsValid(Camera))
			{
				Camera->SetPathRotation(NewRotation);
			}
		}

		return;
	}

	if (AGameCamera* TargetCamera = GetTargetCamera())
	{
		TargetCamera->SetPathRotation(NewRotation);
	}
}

void AGameCameraController::ClearPathRotation()
{
	if (bAffectAllCameras)
	{
		if (CachedCameras.Num() == 0)
		{
			FindAllCameras();
		}

		for (AGameCamera* Camera : CachedCameras)
		{
			if (IsValid(Camera))
			{
				Camera->ClearPathRotation();
			}
		}

		return;
	}

	if (AGameCamera* TargetCamera = GetTargetCamera())
	{
		TargetCamera->ClearPathRotation();
	}
}


//==========================================================
// Interactive Camera
//==========================================================

void AGameCameraController::SetControlMode(
	ECameraControlMode NewControlMode
)
{
	CameraState.ControlMode = NewControlMode;

	ApplyCameraCommand();
}

void AGameCameraController::SetLookMode(
	ECameraLookMode NewLookMode
)
{
	CameraState.LookMode = NewLookMode;

	ApplyCameraCommand();
}

void AGameCameraController::SetMouseSensitivity(
	float NewSensitivity
)
{
	CameraState.MouseSensitivity = FMath::Max(0.f, NewSensitivity);

	ApplyCameraCommand();
}

void AGameCameraController::SetOrbitYaw(
	float NewYaw
)
{
	CameraState.OrbitYaw = NewYaw;

	ApplyCameraCommand();
}

void AGameCameraController::SetOrbitPitch(
	float NewPitch
)
{
	CameraState.OrbitPitch = FMath::Clamp(
		NewPitch,
		CameraState.MinPitch,
		CameraState.MaxPitch
	);

	ApplyCameraCommand();
}

void AGameCameraController::SetOrbitDistance(
	float NewDistance
)
{
	CameraState.OrbitDistance = FMath::Max(1.f, NewDistance);

	ApplyCameraCommand();
}

void AGameCameraController::SetAimDistance(
	float NewDistance
)
{
	CameraState.AimDistance = FMath::Max(0.f, NewDistance);

	ApplyCameraCommand();
}

void AGameCameraController::SetAimHeight(
	float NewHeight
)
{
	CameraState.AimHeight = NewHeight;

	ApplyCameraCommand();
}


//==========================================================
// Target
//==========================================================

void AGameCameraController::SetConnectTarget(
	AActor* NewConnectTarget
)
{
	ConnectTarget = NewConnectTarget;

	if (bAffectAllCameras)
	{
		if (CachedCameras.Num() == 0)
		{
			FindAllCameras();
		}

		for (AGameCamera* Camera : CachedCameras)
		{
			if (IsValid(Camera))
			{
				Camera->SetConnectTarget(NewConnectTarget);
			}
		}

		return;
	}

	if (AGameCamera* TargetCamera = GetTargetCamera())
	{
		TargetCamera->SetConnectTarget(NewConnectTarget);
	}
}

AActor* AGameCameraController::GetConnectTarget() const
{
	return ConnectTarget;
}


//==========================================================
// Camera
//==========================================================

void AGameCameraController::SetCamera(
	AGameCamera* NewCamera
)
{
	GameCamera = NewCamera;
}

void AGameCameraController::SetCameraTargetByTag(
	FName NewCameraTag
)
{
	CameraTag = NewCameraTag;
	bUseCameraTag = !CameraTag.IsNone();
}

FName AGameCameraController::GetCameraTag() const
{
	return CameraTag;
}


//==========================================================
// Presets
//==========================================================

void AGameCameraController::ApplyPreset(
	FName NewPresetName
)
{
	PresetName = NewPresetName;

	if (bAffectAllCameras)
	{
		if (CachedCameras.Num() == 0)
		{
			FindAllCameras();
		}

		for (AGameCamera* Camera : CachedCameras)
		{
			if (IsValid(Camera))
			{
				Camera->ApplyPreset(NewPresetName);
			}
		}

		return;
	}

	if (AGameCamera* TargetCamera = GetTargetCamera())
	{
		TargetCamera->ApplyPreset(NewPresetName);
	}
}


//==========================================================
// Restore
//==========================================================

void AGameCameraController::ApplyRestore()
{
	if (bAffectAllCameras)
	{
		if (CachedCameras.Num() == 0)
		{
			FindAllCameras();
		}

		for (AGameCamera* Camera : CachedCameras)
		{
			if (!IsValid(Camera))
			{
				continue;
			}

			Camera->SetConnectTarget(RestoreConnectTarget);

			Camera->SetCameraState(
				RestoreCameraState,
				RestoreTransitionMode
			);
		}

		return;
	}

	AGameCamera* TargetCamera = GetTargetCamera();

	if (!TargetCamera)
	{
		return;
	}

	TargetCamera->SetConnectTarget(RestoreConnectTarget);

	TargetCamera->SetCameraState(
		RestoreCameraState,
		RestoreTransitionMode
	);
}

void AGameCameraController::ResetCamera()
{
	if (bAffectAllCameras)
	{
		if (CachedCameras.Num() == 0)
		{
			FindAllCameras();
		}

		for (AGameCamera* Camera : CachedCameras)
		{
			if (IsValid(Camera))
			{
				Camera->ResetState();
			}
		}

		return;
	}

	if (AGameCamera* TargetCamera = GetTargetCamera())
	{
		TargetCamera->ResetState();
	}
}


//==========================================================
// Apply All
//==========================================================

void AGameCameraController::ApplyParametersToAllCameras()
{
	if (CachedCameras.Num() == 0)
	{
		FindAllCameras();
	}

	for (AGameCamera* Camera : CachedCameras)
	{
		if (!IsValid(Camera))
		{
			continue;
		}

		ApplyToCamera(
			Camera,
			CameraState,
			ConnectTarget,
			TransitionMode
		);
	}
}


//==========================================================
// Trigger / Eventer
//==========================================================

void AGameCameraController::ActivateEventer(
	AActor* OtherActor
)
{
	if (!IsValid(OtherActor))
	{
		return;
	}

	if (bFireOnlyOnce && bTriggered)
	{
		return;
	}

	PendingActor = OtherActor;

	if (ActivationDelay <= 0.f)
	{
		ActivationTimer = 0.f;
		ApplyCameraCommand();

		if (CameraPathActor)
		{
			StartPath();
		}

		bTriggered = true;
		PendingActor.Reset();

		return;
	}

	ActivationTimer = ActivationDelay;
}

void AGameCameraController::RestoreEventer(
	AActor* OtherActor
)
{
	if (!bRestoreOnExit)
	{
		return;
	}

	if (!IsValid(OtherActor))
	{
		return;
	}

	StopPath();

	if (!RestorePresetName.IsNone())
	{
		ApplyPreset(RestorePresetName);
	}
	else
	{
		ApplyRestore();
	}

	if (RestoreCameraPathActor)
	{
		StartRestorePath();
	}
}


//==========================================================
// Activation Delay
//==========================================================

void AGameCameraController::CheckActivationDelay(
	float DeltaTime
)
{
	if (ActivationTimer <= 0.f)
	{
		return;
	}

	ActivationTimer -= DeltaTime;

	if (ActivationTimer > 0.f)
	{
		return;
	}

	ActivationTimer = 0.f;

	if (!PendingActor.IsValid())
	{
		return;
	}

	ApplyCameraCommand();

	if (CameraPathActor)
	{
		StartPath();
	}

	bTriggered = true;
	PendingActor.Reset();
}


//==========================================================
// Path
//==========================================================

void AGameCameraController::StartPath()
{
	if (!IsValid(CameraPathActor))
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
	ActiveFollowSmoothing = FMath::Max(0.1f, FollowSmoothing);
	bActivePathLoop = bPathLoop;

	CurrentPathDistance = 0.f;
	bPathFinished = false;
	bPathActive = true;

	FindClosestPathDistance(CurrentPathDistance);

	ApplyPathRotation();
}

void AGameCameraController::StopPath()
{
	bPathActive = false;
	bPathFinished = false;
	ActiveSpline = nullptr;
	CurrentPathDistance = 0.f;

	ClearPathRotation();
}

bool AGameCameraController::IsPathActive() const
{
	return bPathActive;
}

bool AGameCameraController::FindClosestPathDistance(
	float& OutDistance
) const
{
	OutDistance = 0.f;

	if (!ActiveSpline)
	{
		return false;
	}

	AGameCamera* TargetCamera = GameCamera;

	if (!IsValid(TargetCamera))
	{
		TargetCamera = nullptr;
	}

	if (!TargetCamera)
	{
		for (AGameCamera* Camera : CachedCameras)
		{
			if (IsValid(Camera))
			{
				TargetCamera = Camera;
				break;
			}
		}
	}

	if (!TargetCamera)
	{
		return false;
	}

	const FVector CameraLocation = TargetCamera->GetActorLocation();

	const float SplineLength = ActiveSpline->GetSplineLength();

	if (SplineLength <= 0.f)
	{
		return false;
	}

	constexpr int32 Samples = 100;

	float BestDistance = 0.f;
	float BestSquaredDistance = TNumericLimits<float>::Max();

	for (int32 Index = 0; Index <= Samples; ++Index)
	{
		const float Distance =
			SplineLength * static_cast<float>(Index) / static_cast<float>(Samples);

		const FVector Location =
			ActiveSpline->GetLocationAtDistanceAlongSpline(
				Distance,
				ESplineCoordinateSpace::World
			);

		const float SquaredDistance =
			FVector::DistSquared(CameraLocation, Location);

		if (SquaredDistance < BestSquaredDistance)
		{
			BestSquaredDistance = SquaredDistance;
			BestDistance = Distance;
		}
	}

	OutDistance = BestDistance;

	return true;
}

void AGameCameraController::UpdatePath(
	float DeltaTime
)
{
	if (!bPathActive || !ActiveSpline)
	{
		return;
	}

	const float SplineLength = ActiveSpline->GetSplineLength();

	if (SplineLength <= 0.f)
	{
		StopPath();
		return;
	}

	const float Speed =
		FMath::Max(
			0.1f,
			ActiveFollowSmoothing
		);

	CurrentPathDistance += Speed * DeltaTime * 100.f;

	if (bActivePathLoop)
	{
		CurrentPathDistance =
			FMath::Fmod(CurrentPathDistance, SplineLength);
	}
	else if (CurrentPathDistance >= SplineLength)
	{
		CurrentPathDistance = SplineLength;
		bPathFinished = true;
		bPathActive = false;
	}

	const FVector Location =
		ActiveSpline->GetLocationAtDistanceAlongSpline(
			CurrentPathDistance,
			ESplineCoordinateSpace::World
		);

	const FVector Direction =
		ActiveSpline->GetDirectionAtDistanceAlongSpline(
			CurrentPathDistance,
			ESplineCoordinateSpace::World
		);

	AGameCamera* TargetCamera = GetTargetCamera();

	if (!TargetCamera)
	{
		return;
	}

	TargetCamera->SetActorLocation(Location);

	if (!Direction.IsNearlyZero())
	{
		const FRotator Rotation =
			Direction.Rotation();

		TargetCamera->SetActorRotation(Rotation);
	}

	ApplyPathRotation();
}

void AGameCameraController::ApplyPathRotation()
{
	if (!ActiveSpline || !bPathActive)
	{
		return;
	}

	const FRotator Rotation =
		ActiveSpline->GetRotationAtDistanceAlongSpline(
			CurrentPathDistance,
			ESplineCoordinateSpace::World
		);

	if (bAffectAllCameras)
	{
		for (AGameCamera* Camera : CachedCameras)
		{
			if (IsValid(Camera))
			{
				Camera->SetPathRotation(Rotation);
			}
		}

		return;
	}

	if (AGameCamera* TargetCamera = GetTargetCamera())
	{
		TargetCamera->SetPathRotation(Rotation);
	}
}


//==========================================================
// Restore Path
//==========================================================

void AGameCameraController::StartRestorePath()
{
	if (!IsValid(RestoreCameraPathActor))
	{
		return;
	}

	USplineComponent* Spline =
		RestoreCameraPathActor->FindComponentByClass<USplineComponent>();

	if (!Spline)
	{
		return;
	}

	ActiveSpline = Spline;

	ActiveStartDistanceThreshold =
		RestoreStartDistanceThreshold;

	ActivePathOffset =
		RestorePathOffset;

	ActiveFollowSmoothing =
		FMath::Max(
			0.1f,
			RestoreFollowSmoothing
		);

	bActivePathLoop = bRestorePathLoop;

	CurrentPathDistance = 0.f;
	bPathFinished = false;
	bPathActive = true;

	FindClosestPathDistance(CurrentPathDistance);

	ApplyPathRotation();
}

void AGameCameraController::StopRestorePath()
{
	StopPath();
}


//==========================================================
// Targets
//==========================================================

void AGameCameraController::ApplyTargets(
	AActor* NewConnectTarget
)
{
	SetConnectTarget(NewConnectTarget);
}


//==========================================================
// Preset Data
//==========================================================

void AGameCameraController::SavePreset()
{
	if (!PresetData)
	{
		return;
	}
}

void AGameCameraController::LoadPreset()
{
	if (!PresetData)
	{
		return;
	}
}

void AGameCameraController::DeletePreset()
{
	if (!PresetData)
	{
		return;
	}
}