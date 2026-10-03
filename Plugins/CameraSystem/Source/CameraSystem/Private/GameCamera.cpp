#include "GameCamera.h"
#include "Engine/World.h"
#include "GameCameraEventer.h"
#include "CollisionQueryParams.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AGameCamera::AGameCamera()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGameCamera::BeginPlay()
{
	Super::BeginPlay();

	if (!PlayerTarget)
	{
		PlayerTarget = GetOwner();
	}

	CurrentState = TargetState;
	TargeFieldOfView = TargetState.FieldOfView;
	FixedWorldPosition = GetWorldPositionFromState(CurrentState);

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		EnableInput(PC);
	}

	if (UCameraComponent* Camera = GetCameraComponent())
	{
		Camera->SetRelativeLocation(FVector::ZeroVector);
		Camera->SetRelativeRotation(FRotator::ZeroRotator);
		Camera->SetFieldOfView(TargeFieldOfView);
	}

	if (CurrentState.ControlMode != ECameraControlMode::None)
	{
		InitializeInteractiveCamera();
	}

	if (CurrentState.ControlMode == ECameraControlMode::FirstPerson && PlayerTarget)
	{
		SetActorLocation(PlayerTarget->GetActorTransform().TransformPosition(CurrentState.Position));
		SetActorRotation(PlayerTarget->GetActorRotation());
	}
}

void AGameCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateBlend(DeltaTime);



	if (UCameraComponent* Camera = GetCameraComponent())
	{
		UpdateFieldOfView(DeltaTime, TargeFieldOfView);
	}


	//const bool bWaitingForFirstPerson = TargetState.ControlMode == ECameraControlMode::FirstPerson && CurrentState.ControlMode != ECameraControlMode::FirstPerson;

	//if (bWaitingForFirstPerson)
	//{
	//	if (UCameraComponent* Camera = GetCameraComponent())
	//	{
	//		Camera->SetFieldOfView(CurrentState.FieldOfView);
	//	}

	//	return;
	//}

	if (CurrentState.ControlMode != ECameraControlMode::None)
	{
		UpdateInteractiveCamera(DeltaTime);
	}
	else
	{
		UpdatePosition(DeltaTime);
		UpdateRotation(DeltaTime);
	}


}

void AGameCamera::SetPlayerTarget(AActor* NewTarget)
{
	PlayerTarget = NewTarget;
	bInteractiveInitialized = false;
}

void AGameCamera::SetConnectTarget(AActor* NewTarget)
{
	ConnectTarget = NewTarget;
	bInteractiveInitialized = false;
}

void AGameCamera::SetState(const FGameCameraState& NewState)
{
	const ECameraControlMode OldControlMode = CurrentState.ControlMode;
	const bool bLeavingFirstPerson = OldControlMode == ECameraControlMode::FirstPerson && NewState.ControlMode != ECameraControlMode::FirstPerson;

	if (CurrentState.PositionMode != NewState.PositionMode)
	{
		ConvertPositionSpace(NewState.PositionMode);
	}

	TargetState = NewState;
	TargeFieldOfView = TargetState.FieldOfView;

	if (NewState.FollowTarget == ECameraFollowTarget::None)
	{
		FixedWorldPosition = GetWorldPositionFromState(NewState);
	}

	if (bLeavingFirstPerson)
	{
		CurrentState.PositionMode = NewState.PositionMode;
		CurrentState.FollowTarget = NewState.FollowTarget;
		CurrentState.ControlMode = NewState.ControlMode;
		CurrentState.LookMode = NewState.LookMode;
		CurrentState.bUseTargetRotation = NewState.bUseTargetRotation;
		CurrentState.bUsePathRotation = NewState.bUsePathRotation;
		CurrentState.bLookAtTarget = NewState.bLookAtTarget;
		CurrentState.bUseCollision = NewState.bUseCollision;
		CurrentState.MovementSpeed = NewState.MovementSpeed;
		CurrentState.RotationSpeed = NewState.RotationSpeed;
		CurrentState.BlendSpeed = NewState.BlendSpeed;
		CurrentState.MouseSensitivity = NewState.MouseSensitivity;
		CurrentState.MinPitch = NewState.MinPitch;
		CurrentState.MaxPitch = NewState.MaxPitch;
		CurrentState.OrbitDistance = NewState.OrbitDistance;
		CurrentState.AimDistance = NewState.AimDistance;
		CurrentState.AimHeight = NewState.AimHeight;
		CurrentState.LockOnRotation = NewState.LockOnRotation;
		CurrentState.OrbitYaw = NewState.OrbitYaw;
		CurrentState.OrbitPitch = NewState.OrbitPitch;

		bInteractiveInitialized = false;
		return;
	}

	if (OldControlMode != NewState.ControlMode)
	{
		bInteractiveInitialized = false;
	}
}

void AGameCamera::SetCameraState(const FGameCameraState& NewState, ECameraTransitionMode TransitionMode)
{
	if (TransitionMode == ECameraTransitionMode::Smooth)
	{
		SetState(NewState);
		return;
	}

	if (TransitionMode == ECameraTransitionMode::Instant)
	{
		const ECameraControlMode OldControlMode = CurrentState.ControlMode;

		if (CurrentState.PositionMode != NewState.PositionMode)
		{
			ConvertPositionSpace(NewState.PositionMode);
		}

		CurrentState = NewState;
		TargetState = NewState;

		if (NewState.FollowTarget == ECameraFollowTarget::None)
		{
			FixedWorldPosition = GetWorldPositionFromState(NewState);
		}

		if (OldControlMode != NewState.ControlMode)
		{
			bInteractiveInitialized = false;
		}

		if (NewState.ControlMode != ECameraControlMode::None)
		{
			if (!bInteractiveInitialized)
			{
				InitializeInteractiveCamera();
			}

			UpdateInteractiveCamera(0.f);
		}
		else
		{
			SetActorLocation(CalculateDesiredLocation());

			FRotator DesiredRotation = NewState.Rotation;

			if (NewState.bUsePathRotation && bPathRotationValid)
			{
				DesiredRotation = PathRotation;
			}
			else if (NewState.bUseTargetRotation)
			{
				if (AActor* RotationTarget = GetRotationTarget())
				{
					DesiredRotation = RotationTarget->GetActorRotation();
				}
			}
			else if (NewState.bLookAtTarget && PlayerTarget)
			{
				DesiredRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), PlayerTarget->GetActorLocation());
			}

			SetActorRotation(DesiredRotation);
		}

		if (UCameraComponent* Camera = GetCameraComponent())
		{
			Camera->SetFieldOfView(NewState.FieldOfView);
			CurrentState.FieldOfView = NewState.FieldOfView;
		}
	}
}

void AGameCamera::SetPositionMode(ECameraPositionMode NewMode)
{
	if (CurrentState.PositionMode == NewMode)
	{
		return;
	}

	ConvertPositionSpace(NewMode);

	CurrentState.PositionMode = NewMode;
	TargetState.PositionMode = NewMode;

	if (CurrentState.FollowTarget == ECameraFollowTarget::None)
	{
		FixedWorldPosition = GetWorldPositionFromState(CurrentState);
	}
}

void AGameCamera::SetCameraTransform(FVector NewPosition, FRotator NewRotation, ECameraPositionMode NewPositionMode)
{
	CurrentState.Position = NewPosition;
	TargetState.Position = NewPosition;
	CurrentState.Rotation = NewRotation;
	TargetState.Rotation = NewRotation;
	CurrentState.PositionMode = NewPositionMode;
	TargetState.PositionMode = NewPositionMode;

	if (CurrentState.FollowTarget == ECameraFollowTarget::None)
	{
		FixedWorldPosition = GetWorldPositionFromState(CurrentState);
	}

	SetActorLocation(CalculateDesiredLocation());
	SetActorRotation(NewRotation);
}

void AGameCamera::SetPathRotation(FRotator NewRotation)
{
	PathRotation = NewRotation;
	bPathRotationValid = true;
}

void AGameCamera::ClearPathRotation()
{
	bPathRotationValid = false;
}

AActor* AGameCamera::GetPositionTarget() const
{
	switch (CurrentState.PositionMode)
	{
	case ECameraPositionMode::ConnectTarget:
		return ConnectTarget;

	case ECameraPositionMode::PlayerTarget:
		return PlayerTarget;

	default:
		return nullptr;
	}
}

AActor* AGameCamera::GetFollowTarget() const
{
	switch (CurrentState.FollowTarget)
	{
	case ECameraFollowTarget::ConnectTarget:
		return ConnectTarget;

	case ECameraFollowTarget::PlayerTarget:
		return PlayerTarget;

	default:
		return nullptr;
	}
}

AActor* AGameCamera::GetRotationTarget() const
{
	if (CurrentState.PositionMode == ECameraPositionMode::ConnectTarget)
	{
		return ConnectTarget;
	}

	if (CurrentState.PositionMode == ECameraPositionMode::PlayerTarget)
	{
		return PlayerTarget;
	}

	return PlayerTarget;
}

AActor* AGameCamera::GetInteractiveTarget() const
{
	if (PlayerTarget)
	{
		return PlayerTarget;
	}

	if (ConnectTarget)
	{
		return ConnectTarget;
	}

	return nullptr;
}

FVector AGameCamera::GetWorldPositionFromState(const FGameCameraState& State) const
{
	if (State.PositionMode == ECameraPositionMode::World)
	{
		return State.Position;
	}

	AActor* Target = nullptr;

	if (State.PositionMode == ECameraPositionMode::ConnectTarget)
	{
		Target = ConnectTarget;
	}
	else if (State.PositionMode == ECameraPositionMode::PlayerTarget)
	{
		Target = PlayerTarget;
	}

	if (!Target)
	{
		return State.Position;
	}

	return Target->GetActorLocation() + State.Position;
}

FVector AGameCamera::GetRelativePositionFromWorld(const FVector& WorldPosition, ECameraPositionMode PositionMode) const
{
	AActor* Target = nullptr;

	if (PositionMode == ECameraPositionMode::ConnectTarget)
	{
		Target = ConnectTarget;
	}
	else if (PositionMode == ECameraPositionMode::PlayerTarget)
	{
		Target = PlayerTarget;
	}

	if (!Target)
	{
		return WorldPosition;
	}

	return Target->GetActorTransform().InverseTransformPosition(WorldPosition);
}

void AGameCamera::ConvertPositionSpace(ECameraPositionMode NewPositionMode)
{
	const FVector WorldPosition = GetActorLocation();
	const FRotator WorldRotation = GetActorRotation();

	if (NewPositionMode == ECameraPositionMode::World)
	{
		CurrentState.Position = WorldPosition;
	}
	else if (NewPositionMode == ECameraPositionMode::PlayerTarget)
	{
		if (PlayerTarget)
		{
			CurrentState.Position = PlayerTarget->GetActorTransform().InverseTransformPosition(WorldPosition);
		}
	}
	else if (NewPositionMode == ECameraPositionMode::ConnectTarget)
	{
		if (ConnectTarget)
		{
			CurrentState.Position = ConnectTarget->GetActorTransform().InverseTransformPosition(WorldPosition);
		}
	}

	CurrentState.Rotation = WorldRotation;

	if (CurrentState.FollowTarget == ECameraFollowTarget::None)
	{
		FixedWorldPosition = WorldPosition;
	}
}

void AGameCamera::ResetState()
{
	SetCameraState(FGameCameraState(), ECameraTransitionMode::Smooth);
}

void AGameCamera::ApplyPreset(FName PresetName)
{
	for (const FGameCameraPreset& Preset : Presets)
	{
		if (Preset.Name == PresetName)
		{
			SetCameraState(Preset.State, ECameraTransitionMode::Smooth);
			return;
		}
	}
}

void AGameCamera::UpdateBlend(float DeltaTime)
{
	const float Speed = FMath::Max(CurrentState.BlendSpeed, 0.01f);
	const bool bFirstPerson = TargetState.ControlMode == ECameraControlMode::FirstPerson;
	const bool bThirdPerson = TargetState.ControlMode == ECameraControlMode::ThirdPerson;
	const bool bHasConnectTarget = ConnectTarget != nullptr;
	const bool bConnectThirdPerson = bThirdPerson && bHasConnectTarget;

	if (bFirstPerson && CurrentState.ControlMode != ECameraControlMode::FirstPerson)
	{
		FVector TargetWorldPosition = TargetState.Position;
		FRotator TargetWorldRotation = TargetState.Rotation;

		if (PlayerTarget)
		{
			TargetWorldPosition = PlayerTarget->GetActorTransform().TransformPosition(TargetState.Position);
			TargetWorldRotation = PlayerTarget->GetActorRotation();
		}

		CurrentState.BlendSpeed = FMath::FInterpTo(CurrentState.BlendSpeed, 30.f, DeltaTime, 3.f);

		const float FirstPersonSpeed = FMath::Max(CurrentState.BlendSpeed, 0.01f);
		const float Distance = FVector::Dist(GetActorLocation(), TargetWorldPosition);

		FVector NewWorldPosition;

		if (Distance <= 50.f)
		{
			NewWorldPosition = TargetWorldPosition;
		}
		else
		{
			NewWorldPosition = FMath::VInterpTo(GetActorLocation(), TargetWorldPosition, DeltaTime, FirstPersonSpeed);
		}

		const float RotationDistance = GetActorRotation().GetManhattanDistance(TargetWorldRotation);

		FRotator NewWorldRotation;

		if (RotationDistance <= 10.f)
		{
			NewWorldRotation = TargetWorldRotation;
		}
		else
		{
			NewWorldRotation = FMath::RInterpTo(GetActorRotation(), TargetWorldRotation, DeltaTime, FirstPersonSpeed);
		}

		SetActorLocation(NewWorldPosition);
		SetActorRotation(NewWorldRotation);


		const bool bPositionReached = NewWorldPosition.Equals(TargetWorldPosition, 1.f);
		const bool bRotationReached = NewWorldRotation.Equals(TargetWorldRotation, 1.f);

		if (!bPositionReached || !bRotationReached)
		{
			CurrentState.ControlMode = ECameraControlMode::None;
			CurrentState.FollowTarget = ECameraFollowTarget::None;
			CurrentState.bUseTargetRotation = false;
			CurrentState.bUsePathRotation = false;
			CurrentState.bLookAtTarget = false;
			CurrentState.bUseCollision = false;
			FixedWorldPosition = NewWorldPosition;
			return;
		}

		SetActorLocation(TargetWorldPosition);
		SetActorRotation(TargetWorldRotation);

		CurrentState = TargetState;

		CurrentState.OrbitYaw = GetActorRotation().Yaw;
		CurrentState.OrbitPitch = FMath::Clamp(GetActorRotation().Pitch, CurrentState.MinPitch, CurrentState.MaxPitch);
		bInteractiveInitialized = true;

		return;
	}

	CurrentState.Position = FMath::VInterpTo(CurrentState.Position, TargetState.Position, DeltaTime, CurrentState.BlendSpeed);
	CurrentState.Rotation = FMath::RInterpTo(CurrentState.Rotation, TargetState.Rotation, DeltaTime, CurrentState.BlendSpeed);
	CurrentState.PositionMode = TargetState.PositionMode;
	CurrentState.FollowTarget = TargetState.FollowTarget;
	CurrentState.bUseTargetRotation = TargetState.bUseTargetRotation;
	CurrentState.bUsePathRotation = TargetState.bUsePathRotation;
	CurrentState.bLookAtTarget = TargetState.bLookAtTarget;
	CurrentState.bUseCollision = TargetState.bUseCollision;
	CurrentState.MovementSpeed = TargetState.MovementSpeed;
	CurrentState.RotationSpeed = TargetState.RotationSpeed;
	CurrentState.BlendSpeed = TargetState.BlendSpeed;
	CurrentState.ControlMode = TargetState.ControlMode;
	CurrentState.LookMode = TargetState.LookMode;
	CurrentState.MouseSensitivity = TargetState.MouseSensitivity;
	CurrentState.MinPitch = TargetState.MinPitch;
	CurrentState.MaxPitch = TargetState.MaxPitch;


	if (!bConnectThirdPerson)
	{
		CurrentState.OrbitDistance = TargetState.OrbitDistance;
		CurrentState.AimDistance = TargetState.AimDistance;
		CurrentState.AimHeight = TargetState.AimHeight;

		if (!bInteractiveInitialized)
		{
			CurrentState.LockOnRotation = TargetState.LockOnRotation;
			CurrentState.OrbitYaw = TargetState.OrbitYaw;
			CurrentState.OrbitPitch = TargetState.OrbitPitch;
		}
	}
	else
	{
		CurrentState.OrbitDistance = TargetState.OrbitDistance;
		CurrentState.AimDistance = TargetState.AimDistance;
		CurrentState.AimHeight = TargetState.AimHeight;
		CurrentState.LockOnRotation = TargetState.LockOnRotation;
		CurrentState.OrbitYaw = TargetState.OrbitYaw;
		CurrentState.OrbitPitch = TargetState.OrbitPitch;
	}

	if (CurrentState.ControlMode != ECameraControlMode::None && !bInteractiveInitialized)
	{
		InitializeInteractiveCamera();
	}
}

void AGameCamera::UpdateFieldOfView(float DeltaTime, float TargetFOV)
{
	UCameraComponent* Camera = GetCameraComponent();

	if (!Camera)
	{
		return;
	}


	const float CurrentFOV = Camera->FieldOfView;
	UE_LOG(LogTemp, Warning, TEXT("FOV %f -> %f speed %f"), CurrentFOV, TargetFOV, TargetState.BlendSpeed);

	const float NewFOV = FMath::FInterpTo(
		CurrentFOV,
		TargetFOV,
		DeltaTime,
		TargetState.BlendSpeed
	);

	Camera->SetFieldOfView(NewFOV);
	CurrentState.FieldOfView = NewFOV;
}

FVector AGameCamera::CalculateDesiredLocation() const
{
	if (CurrentState.FollowTarget == ECameraFollowTarget::None)
	{
		return FixedWorldPosition;
	}

	AActor* FollowTarget = GetFollowTarget();

	if (!FollowTarget)
	{
		return FixedWorldPosition;
	}

	const FVector TargetLocation = FollowTarget->GetActorLocation();
	const FVector DesiredLocation = TargetLocation + CurrentState.Position;

	if (CurrentState.ControlMode != ECameraControlMode::None)
	{
		return DesiredLocation;
	}

	if (CurrentState.PositionMode != ECameraPositionMode::PlayerTarget)
	{
		return DesiredLocation;
	}

	if (CurrentState.FollowTarget != ECameraFollowTarget::PlayerTarget)
	{
		return DesiredLocation;
	}

	if (!CurrentState.bUseCollision)
	{
		return DesiredLocation;
	}

	FHitResult Hit;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(GameCameraNormalCollision), false);
	Params.AddIgnoredActor(FollowTarget);
	Params.AddIgnoredActor(this);

	const FCollisionShape CollisionShape = FCollisionShape::MakeSphere(15.f);

	const bool bHit = GetWorld()->SweepSingleByChannel(
		Hit,
		TargetLocation,
		DesiredLocation,
		FQuat::Identity,
		ECC_Camera,
		CollisionShape,
		Params
	);

	if (bHit)
	{
		return Hit.Location + Hit.Normal * 15.f;
	}

	return DesiredLocation;
}

FVector AGameCamera::CalculateConnectTargetCameraLocation() const
{
	if (!PlayerTarget || !ConnectTarget)
	{
		return PlayerTarget ? PlayerTarget->GetActorLocation() : GetActorLocation();
	}

	const FVector PlayerLocation = PlayerTarget->GetActorLocation();
	const FVector ConnectLocation = ConnectTarget->GetActorLocation();

	FVector ConnectToPlayer = PlayerLocation - ConnectLocation;
	ConnectToPlayer.Z = 0.f;

	if (ConnectToPlayer.IsNearlyZero())
	{
		ConnectToPlayer = PlayerTarget->GetActorForwardVector();
		ConnectToPlayer.Z = 0.f;
	}

	ConnectToPlayer.Normalize();

	const FRotator LockRotation(0.f, CurrentState.LockOnRotation, 0.f);
	const FVector CameraDirection = LockRotation.RotateVector(ConnectToPlayer).GetSafeNormal2D();

	const float PitchRadians = FMath::DegreesToRadians(CurrentState.OrbitPitch);
	const float HorizontalDistance = CurrentState.OrbitDistance * FMath::Cos(PitchRadians);
	const float VerticalOffset = CurrentState.OrbitDistance * FMath::Sin(PitchRadians);
	const FVector HorizontalDirection = CameraDirection.GetSafeNormal2D();

	return PlayerLocation + HorizontalDirection * HorizontalDistance + FVector(0.f, 0.f, VerticalOffset);
}

FVector AGameCamera::CalculateInteractiveLocation() const
{
	AActor* Player = PlayerTarget;

	if (!Player)
	{
		Player = GetInteractiveTarget();
	}

	if (!Player)
	{
		return GetActorLocation();
	}

	if (CurrentState.ControlMode == ECameraControlMode::FirstPerson)
	{
		return Player->GetActorTransform().TransformPosition(CurrentState.Position);
	}

	if (CurrentState.ControlMode == ECameraControlMode::ThirdPerson && CurrentState.LookMode == ECameraLookMode::LookAtAim && ConnectTarget)
	{
		return CalculateConnectTargetCameraLocation();
	}

	const FVector PlayerLocation = Player->GetActorLocation();
	const FRotator OrbitRotation(0.f, -CurrentState.OrbitYaw, 0.f);
	const FVector HorizontalDirection = OrbitRotation.Vector();
	const float PitchRadians = FMath::DegreesToRadians(CurrentState.OrbitPitch);

	const FVector OrbitDirection(
		HorizontalDirection.X * FMath::Cos(PitchRadians),
		HorizontalDirection.Y * FMath::Cos(PitchRadians),
		FMath::Sin(PitchRadians)
	);

	const FVector DesiredLocation = PlayerLocation + OrbitDirection * CurrentState.OrbitDistance;

	FHitResult Hit;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(GameCamera), false);
	Params.AddIgnoredActor(Player);
	Params.AddIgnoredActor(this);

	const bool bHit = GetWorld()->SweepSingleByChannel(
		Hit,
		PlayerLocation,
		DesiredLocation,
		FQuat::Identity,
		ECC_Camera,
		FCollisionShape::MakeSphere(15.f),
		Params
	);

	if (bHit)
	{
		return Hit.Location + Hit.Normal * 15.f;
	}

	return DesiredLocation;
}

FVector AGameCamera::CalculateAimPoint() const
{
	AActor* Player = PlayerTarget;

	if (!Player)
	{
		Player = GetInteractiveTarget();
	}

	if (!Player)
	{
		return FVector::ZeroVector;
	}

	if (CurrentState.LookMode == ECameraLookMode::LookAtTarget)
	{
		if (ConnectTarget)
		{
			return ConnectTarget->GetActorLocation() + FVector(0.f, 0.f, CurrentState.AimHeight);
		}

		return Player->GetActorLocation() + FVector(0.f, 0.f, CurrentState.AimHeight);
	}

	const FVector PlayerLocation = Player->GetActorLocation();

	FVector Forward = Player->GetActorForwardVector();
	Forward.Z = 0.f;
	Forward.Normalize();

	return PlayerLocation + FVector(0.f, 0.f, CurrentState.AimHeight) + Forward * CurrentState.AimDistance;
}

void AGameCamera::InitializeInteractiveCamera()
{
	if (CurrentState.ControlMode == ECameraControlMode::None)
	{
		bInteractiveInitialized = false;
		return;
	}

	AActor* Target = GetInteractiveTarget();

	if (!Target)
	{
		return;
	}

	if (CurrentState.ControlMode == ECameraControlMode::ThirdPerson)
	{
		if (ConnectTarget && CurrentState.LookMode == ECameraLookMode::LookAtAim)
		{
			const FVector PlayerLocation = Target->GetActorLocation();
			const FVector ConnectLocation = ConnectTarget->GetActorLocation();

			FVector BaseDirection = PlayerLocation - ConnectLocation;
			BaseDirection.Z = 0.f;

			if (!BaseDirection.IsNearlyZero())
			{
				BaseDirection.Normalize();

				FVector CurrentCameraDirection = GetActorLocation() - PlayerLocation;
				CurrentCameraDirection.Z = 0.f;

				if (!CurrentCameraDirection.IsNearlyZero())
				{
					CurrentCameraDirection.Normalize();

					const float BaseYaw = BaseDirection.Rotation().Yaw;
					const float CameraYaw = CurrentCameraDirection.Rotation().Yaw;

					CurrentState.LockOnRotation = FMath::FindDeltaAngleDegrees(BaseYaw, CameraYaw);
				}
			}

			const FVector Relative = GetActorLocation() - PlayerLocation;
			const float Distance = Relative.Size();

			if (Distance > 1.f)
			{
				CurrentState.OrbitDistance = Distance;
			}

			const float HorizontalDistance = FVector2D(Relative.X, Relative.Y).Size();

			if (HorizontalDistance > 1.f)
			{
				CurrentState.OrbitPitch = FMath::RadiansToDegrees(FMath::Atan2(Relative.Z, HorizontalDistance));
			}
		}
		else
		{
			const FVector RelativeLocation = GetActorLocation() - Target->GetActorLocation();
			const float Distance = RelativeLocation.Size();

			if (Distance > 1.f)
			{
				CurrentState.OrbitDistance = Distance;
			}

			const FVector Horizontal(RelativeLocation.X, RelativeLocation.Y, 0.f);

			if (!Horizontal.IsNearlyZero())
			{
				CurrentState.OrbitYaw = Horizontal.Rotation().Yaw;
			}

			const float HorizontalDistance = FVector2D(RelativeLocation.X, RelativeLocation.Y).Size();

			if (HorizontalDistance > 1.f)
			{
				CurrentState.OrbitPitch = FMath::RadiansToDegrees(FMath::Atan2(RelativeLocation.Z, HorizontalDistance));
			}
		}
	}
	else if (CurrentState.ControlMode == ECameraControlMode::FirstPerson)
	{
		CurrentState.OrbitYaw = GetActorRotation().Yaw;
		CurrentState.OrbitPitch = GetActorRotation().Pitch;

		if (FMath::IsNearlyZero(CurrentState.OrbitYaw) && FMath::IsNearlyZero(CurrentState.OrbitPitch))
		{
			CurrentState.OrbitYaw = Target->GetActorRotation().Yaw;
			CurrentState.OrbitPitch = Target->GetActorRotation().Pitch;
		}
	}

	CurrentState.OrbitPitch = FMath::Clamp(CurrentState.OrbitPitch, CurrentState.MinPitch, CurrentState.MaxPitch);
	bInteractiveInitialized = true;
}

void AGameCamera::UpdateInteractiveCamera(float DeltaTime)
{
	AActor* Player = PlayerTarget;

	if (!Player)
	{
		Player = GetInteractiveTarget();
	}

	if (!Player)
	{
		return;
	}

	if (!bInteractiveInitialized)
	{
		InitializeInteractiveCamera();
	}

	if (CurrentState.ControlMode == ECameraControlMode::FirstPerson)
	{
		const FVector CameraLocation = Player->GetActorTransform().TransformPosition(CurrentState.Position);

		SetActorLocation(CameraLocation);

		if (CurrentState.LookMode == ECameraLookMode::LookAtTarget)
		{
			const FVector LookTarget = CalculateAimPoint();

			if (!LookTarget.IsNearlyZero())
			{
				const FRotator DesiredRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), LookTarget);
				const FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), DesiredRotation, DeltaTime, CurrentState.RotationSpeed);
				SetActorRotation(NewRotation);
			}
		}
		else
		{
			const FRotator CameraRotation(CurrentState.OrbitPitch, CurrentState.OrbitYaw, 0.f);
			SetActorRotation(CameraRotation);
		}

		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);

	const bool bConnectAimMode = CurrentState.ControlMode == ECameraControlMode::ThirdPerson && CurrentState.LookMode == ECameraLookMode::LookAtAim && ConnectTarget != nullptr;

	float MouseX = 0.f;
	float MouseY = 0.f;

	if (PC)
	{
		PC->GetInputMouseDelta(MouseX, MouseY);

		CurrentState.OrbitYaw -= MouseX * CurrentState.MouseSensitivity;
		CurrentState.OrbitPitch -= MouseY * CurrentState.MouseSensitivity;
		CurrentState.OrbitPitch = FMath::Clamp(CurrentState.OrbitPitch, CurrentState.MinPitch, CurrentState.MaxPitch);
	}

	if (bConnectAimMode)
	{
		const FVector PlayerLocation = Player->GetActorLocation();
		const FVector ConnectLocation = ConnectTarget->GetActorLocation();

		FVector BaseDirection = PlayerLocation - ConnectLocation;
		BaseDirection.Z = 0.f;

		if (BaseDirection.IsNearlyZero())
		{
			BaseDirection = Player->GetActorForwardVector();
			BaseDirection.Z = 0.f;
		}

		BaseDirection.Normalize();

		const FRotator LockRotation(0.f, CurrentState.LockOnRotation, 0.f);

		FVector CameraDirection = LockRotation.RotateVector(BaseDirection);

		const FRotator MouseRotation(0.f, CurrentState.OrbitYaw, 0.f);

		CameraDirection = MouseRotation.RotateVector(CameraDirection);
		CameraDirection.Normalize();

		const float PitchRadians = FMath::DegreesToRadians(CurrentState.OrbitPitch);
		const float HorizontalDistance = CurrentState.OrbitDistance * FMath::Cos(PitchRadians);
		const float VerticalOffset = CurrentState.OrbitDistance * FMath::Sin(PitchRadians);

		const FVector DesiredLocation = PlayerLocation + CameraDirection * HorizontalDistance + FVector(0.f, 0.f, VerticalOffset);

		const FVector NewLocation = FMath::VInterpTo(GetActorLocation(), DesiredLocation, DeltaTime, CurrentState.MovementSpeed);

		SetActorLocation(NewLocation);

		const FVector LookTarget = CalculateAimPoint();

		if (!LookTarget.IsNearlyZero())
		{
			const FRotator DesiredRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), LookTarget);
			const FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), DesiredRotation, DeltaTime, CurrentState.RotationSpeed);
			SetActorRotation(NewRotation);
		}

		return;
	}

	const FVector DesiredLocation = CalculateInteractiveLocation();
	const FVector NewLocation = FMath::VInterpTo(GetActorLocation(), DesiredLocation, DeltaTime, CurrentState.MovementSpeed);

	SetActorLocation(NewLocation);

	const FVector LookTarget = CalculateAimPoint();

	if (LookTarget.IsNearlyZero())
	{
		return;
	}

	const FRotator DesiredRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), LookTarget);

	SetActorRotation(FMath::RInterpTo(GetActorRotation(), DesiredRotation, DeltaTime, CurrentState.RotationSpeed));
}

void AGameCamera::UpdatePosition(float DeltaTime)
{
	const FVector DesiredLocation = CalculateDesiredLocation();

	SetActorLocation(FMath::VInterpTo(GetActorLocation(), DesiredLocation, DeltaTime, CurrentState.MovementSpeed));
}

void AGameCamera::UpdateRotation(float DeltaTime)
{
	FRotator DesiredRotation = CurrentState.Rotation;

	if (CurrentState.bUsePathRotation && bPathRotationValid)
	{
		DesiredRotation = PathRotation;
	}
	else if (CurrentState.bUseTargetRotation)
	{
		if (AActor* RotationTarget = GetRotationTarget())
		{
			DesiredRotation = RotationTarget->GetActorRotation();
		}
	}
	else if (CurrentState.bLookAtTarget && PlayerTarget)
	{
		DesiredRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), PlayerTarget->GetActorLocation());
	}

	SetActorRotation(FMath::RInterpTo(GetActorRotation(), DesiredRotation, DeltaTime, TargetState.RotationSpeed));
}

void AGameCamera::SetLastTriggeredEventer(AGameCameraEventer* Eventer)
{
	LastTriggeredEventer = Eventer;
}

void AGameCamera::ClearLastTriggeredEventer(AGameCameraEventer* Eventer)
{
	if (LastTriggeredEventer.Get() == Eventer)
	{
		LastTriggeredEventer = nullptr;
	}
}

bool AGameCamera::IsLastTriggeredEventer(AGameCameraEventer* Eventer) const
{
	return LastTriggeredEventer.Get() == Eventer;
}