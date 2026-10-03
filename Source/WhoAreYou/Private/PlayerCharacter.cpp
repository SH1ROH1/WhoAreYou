#include "PlayerCharacter.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "GameCamera.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CollisionQueryParams.h"
#include "WorldCollision.h"
#include "Kismet/GameplayStatics.h"

#include "AbilityListWidget.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"

#include "DrawDebugHelpers.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	if (MovementComponent)
	{
		DefaultGravityScale = MovementComponent->GravityScale;
		DefaultAirControl = MovementComponent->AirControl;
		DefaultBrakingDecelerationWalking = MovementComponent->BrakingDecelerationWalking;
		DefaultGroundFriction = MovementComponent->GroundFriction;

		if (bDashCurse && bDashEnabled)
		{
			bDashIceActive = true;
			MovementComponent->GroundFriction = DashIceGroundFriction;
			MovementComponent->BrakingDecelerationWalking = DashIceBrakingDeceleration;
		}
	}

	for (AActor* Actor : VisionActors)
	{
		if (IsValid(Actor))
		{
			Actor->SetActorHiddenInGame(true);
		}
	}

	for (AActor* Actor : FalseVisionActors)
	{
		if (IsValid(Actor))
		{
			Actor->SetActorHiddenInGame(true);
		}
	}
	if (bShowAbilityList && AbilityListWidgetClass && IsLocallyControlled())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			AbilityListWidget = CreateWidget<UAbilityListWidget>(PlayerController, AbilityListWidgetClass);

			if (AbilityListWidget)
			{
				AbilityListWidget->SetPlayerCharacter(this);
				AbilityListWidget->AddToViewport(10);
			}
		}
	}

}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (CameraActor)
	{
		CameraActor->CurrentState.MouseSensitivity = MouseSensitivity;
	}

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	if (CuurentInteractCooldown > 0.f)
	{
		CuurentInteractCooldown -= DeltaTime;
	}

	if (CurrentDashCooldown > 0.f)
	{
		CurrentDashCooldown -= DeltaTime;
	}

	if (!MovementComponent)
	{
		return;
	}

	if (bJumpHeld && MovementComponent->IsMovingOnGround())
	{
		bJumpHeld = false;
	}

	if (bJumpHeld && MovementComponent->IsFalling() && JumpHoldTime < MaxJumpHoldTime && MovementComponent->Velocity.Z > 0.f)
	{
		const float RemainingTime = MaxJumpHoldTime - JumpHoldTime;
		const float HoldTime = FMath::Min(DeltaTime, RemainingTime);

		FVector Velocity = MovementComponent->Velocity;
		Velocity.Z += JumpHoldForce * HoldTime;
		MovementComponent->Velocity = Velocity;

		JumpHoldTime += HoldTime;

		if (JumpHoldTime >= MaxJumpHoldTime)
		{
			bJumpHeld = false;
		}
	}
	if (bHighJumpEnabled && bHighJumpCurse && MovementComponent->IsFalling())
	{
		const float HeightFromJump = GetActorLocation().Z - JumpStartHeight;

		if (!bHighJumpFallFOVActive && HeightFromJump >= HighJumpCurseHeight && MovementComponent->Velocity.Z < -10.f)
		{
			SetHighJumpCurseFOV();
			bHighJumpFallFOVActive = true;
		}
	}

	if (bGliding && MovementComponent->IsFalling())
	{
		FVector Velocity = MovementComponent->Velocity;
		const float MaxFallSpeed = bGlideCurse ? CursedGlideFallSpeed : GlideFallSpeed;

		if (Velocity.Z < -MaxFallSpeed)
		{
			Velocity.Z = -MaxFallSpeed;
		}

		if (bGlideCurse)
		{
			Velocity.X *= 0.97f;
			Velocity.Y *= 0.97f;
		}

		MovementComponent->Velocity = Velocity;
	}

	if (bGrabbingLedge)
	{
		FVector Forward = CameraActor ? CameraActor->GetActorForwardVector() : GetActorForwardVector();
		FVector Right = CameraActor ? CameraActor->GetActorRightVector() : GetActorRightVector();

		Forward.Z = 0.f;
		Right.Z = 0.f;

		Forward.Normalize();
		Right.Normalize();

		const FVector WallCheckStart = GetActorLocation();
		const FVector WallCheckEnd = WallCheckStart - GrabWallNormal * GrabWallCheckDistance;

		FHitResult WallHit;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		const bool bWallStillExists = GetWorld()->LineTraceSingleByChannel(WallHit, WallCheckStart, WallCheckEnd, ECC_Visibility, QueryParams);

		if (!bWallStillExists || FMath::Abs(WallHit.Normal.Z) > 0.2f)
		{
			OnGrabReleased();
			return;
		}

		GrabWallNormal = WallHit.Normal.GetSafeNormal();
		GrabWallPoint = WallHit.ImpactPoint;

		FVector InputDirection = Forward * MoveX + Right * MoveZ;

		InputDirection -= GrabWallNormal * FVector::DotProduct(InputDirection, GrabWallNormal);
		InputDirection.Z = 0.f;

		if (!InputDirection.IsNearlyZero())
		{
			InputDirection.Normalize();

			if (bLedgeGrabCurse)
			{
				InputDirection *= 0.4f;
			}

			const FVector NewLocation = GetActorLocation() + InputDirection * GrabMoveSpeed * DeltaTime;
			SetActorLocation(NewLocation, true);
		}

		SetActorRotation(FRotator(0.f, (-GrabWallNormal).Rotation().Yaw, 0.f));
		MovementComponent->Velocity = FVector::ZeroVector;
	}
}


bool APlayerCharacter::IsAbilityEnabled(EPlayerAbility Ability) const
{
	switch (Ability)
	{
	case EPlayerAbility::Dash: return bDashEnabled;
	case EPlayerAbility::HighJump: return bHighJumpEnabled;
	case EPlayerAbility::DoubleJump: return bDoubleJumpEnabled;
	case EPlayerAbility::LedgeGrab: return bLedgeGrabEnabled;
	case EPlayerAbility::Vision: return bVisionEnabled;
	case EPlayerAbility::Glide: return bGlideEnabled;
	default: return false;
	}
}

bool APlayerCharacter::IsCurseEnabled(EPlayerCurse Curse) const
{
	switch (Curse)
	{
	case EPlayerCurse::Dash: return bDashCurse;
	case EPlayerCurse::HighJump: return bHighJumpCurse;
	case EPlayerCurse::DoubleJump: return bDoubleJumpCurse;
	case EPlayerCurse::LedgeGrab: return bLedgeGrabCurse;
	case EPlayerCurse::Vision: return bVisionCurse;
	case EPlayerCurse::Glide: return bGlideCurse;
	default: return false;
	}
}


void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerCharacter::OnInteractPressed);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APlayerCharacter::OnSprintPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayerCharacter::OnSprintReleased);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &APlayerCharacter::OnCrouchPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &APlayerCharacter::OnCrouchReleased);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::OnJumpPressed);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &APlayerCharacter::OnJumpReleased);

	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &APlayerCharacter::OnDashPressed);

	PlayerInputComponent->BindAction("Grab", IE_Pressed, this, &APlayerCharacter::OnGrabPressed);
	PlayerInputComponent->BindAction("Grab", IE_Released, this, &APlayerCharacter::OnGrabReleased);

	PlayerInputComponent->BindAction("Vision", IE_Pressed, this, &APlayerCharacter::OnVisionPressed);
	PlayerInputComponent->BindAction("Vision", IE_Released, this, &APlayerCharacter::OnVisionReleased);

	PlayerInputComponent->BindAction("Glide", IE_Pressed, this, &APlayerCharacter::OnGlidePressed);
	PlayerInputComponent->BindAction("Glide", IE_Released, this, &APlayerCharacter::OnGlideReleased);

	PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &APlayerCharacter::OnPausePressed);
	PlayerInputComponent->BindAction("Restart", IE_Pressed, this, &APlayerCharacter::OnRestartPressed);
}

void APlayerCharacter::OnInteractPressed()
{
	if (CuurentInteractCooldown > 0.f || !CameraActor)
	{
		return;
	}

	CuurentInteractCooldown = InteractCooldown;

	const FVector Start = CameraActor->GetActorLocation();
	const FVector Direction = CameraActor->GetActorForwardVector();
	const FVector End = Start + Direction * InteractTraceDistance;

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams))
	{
		AActor* HitActor = Hit.GetActor();

		if (HitActor)
		{
			UGameplayStatics::ApplyPointDamage(HitActor, 1.f, Direction, Hit, GetController(), this, UDamageType::StaticClass());
		}
	}
	UGameplayStatics::PlaySound2D(this, InteractSound);

	//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 0.1f, 0, 1.f);
}

void APlayerCharacter::OnSprintPressed()
{
	MoveSpeed = 800.f;

	if (CameraActor)
	{
		CameraActor->TargeFieldOfView = 120.f;
	}
}

void APlayerCharacter::OnSprintReleased()
{
	MoveSpeed = 600.f;

	if (CameraActor)
	{
		CameraActor->TargeFieldOfView = 100.f;
	}
}

void APlayerCharacter::OnCrouchPressed()
{
	Crouch();
}

void APlayerCharacter::OnCrouchReleased()
{
	UnCrouch();
}

void APlayerCharacter::OnJumpPressed()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	if (!MovementComponent)
	{
		return;
	}

	if (bGrabbingLedge)
	{
		OnGrabReleased();
		return;
	}

	if (bDoubleJumpEnabled && MovementComponent->IsFalling() && AirJumpCount < 1)
	{
		float JumpPower = DoubleJumpVelocity;

		if (bDoubleJumpCurse)
		{
			MovementComponent->AirControl = CursedDoubleJumpAirControl;
		}

		LaunchCharacter(FVector(0.f, 0.f, JumpPower), false, true);

		AirJumpCount++;
		bJumpHeld = false;
		JumpHoldTime = 0.f;

		return;
	}

	if (MovementComponent->IsMovingOnGround())
	{
		float CurrentJumpVelocity = JumpVelocity;

		if (bHighJumpEnabled)
		{
			CurrentJumpVelocity *= HighJumpMultiplier;
		}

		LaunchCharacter(FVector(0.f, 0.f, CurrentJumpVelocity), false, true);

		JumpStartHeight = GetActorLocation().Z;
		AirJumpCount = 0;
		bJumpHeld = true;
		JumpHoldTime = 0.f;
	}
}

void APlayerCharacter::OnJumpReleased()
{
	bJumpHeld = false;

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	if (!MovementComponent || MovementComponent->Velocity.Z <= 0.f)
	{
		return;
	}

	FVector Velocity = MovementComponent->Velocity;
	Velocity.Z *= 0.35f;
	MovementComponent->Velocity = Velocity;
}

void APlayerCharacter::OnDashPressed()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	if (!bDashEnabled || !MovementComponent || CurrentDashCooldown > 0.f)
	{
		return;
	}

	FVector Direction = CameraActor ? CameraActor->GetActorForwardVector() : GetActorForwardVector();

	Direction.Z = 0.f;
	Direction.Normalize();

	LaunchCharacter(Direction * DashStrength, true, false);

	CurrentDashCooldown = DashCooldown;

	if (bDashCurse && !bDashIceActive)
	{
		bDashIceActive = true;
		MovementComponent->GroundFriction = DashIceGroundFriction;
		MovementComponent->BrakingDecelerationWalking = DashIceBrakingDeceleration;
	}
}

void APlayerCharacter::OnGrabPressed()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	if (!bLedgeGrabEnabled || bGrabbingLedge || !MovementComponent || !MovementComponent->IsFalling())
	{
		return;
	}

	const FVector Start = GetActorLocation();
	FVector Direction = CameraActor ? CameraActor->GetActorForwardVector() : GetActorForwardVector();

	Direction.Normalize();

	float CurrentGrabDistance = GrabDistance;

	if (bLedgeGrabCurse)
	{
		CurrentGrabDistance *= 0.5f;
	}

	const FVector End = Start + Direction * CurrentGrabDistance;

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams))
	{
		return;
	}

	if (FMath::Abs(Hit.Normal.Z) > 0.2f)
	{
		return;
	}

	GrabWallNormal = Hit.Normal.GetSafeNormal();
	GrabWallPoint = Hit.ImpactPoint;

	FVector NewLocation = Hit.ImpactPoint + GrabWallNormal * GrabWallOffset;
	NewLocation.Z = GetActorLocation().Z;

	SetActorLocation(NewLocation, true);

	GrabStartRotation = GetActorRotation();

	SetActorRotation(FRotator(0.f, (-GrabWallNormal).Rotation().Yaw, 0.f));

	MovementComponent->SetMovementMode(MOVE_Flying);
	MovementComponent->Velocity = FVector::ZeroVector;

	bGrabbingLedge = true;
}

void APlayerCharacter::OnGrabReleased()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	if (!bGrabbingLedge || !MovementComponent)
	{
		return;
	}

	bGrabbingLedge = false;

	MovementComponent->SetMovementMode(MOVE_Falling);

	const FVector PushDirection = GrabWallNormal * GrabPushOffStrength + FVector(0.f, 0.f, GrabPushOffUpward);

	LaunchCharacter(PushDirection, true, true);

	GrabWallNormal = FVector::ZeroVector;
	GrabWallPoint = FVector::ZeroVector;
}

void APlayerCharacter::OnVisionPressed()
{
	if (!bVisionEnabled)
	{
		return;
	}

	TArray<AActor*>& ActorsToReveal = bVisionCurse ? FalseVisionActors : VisionActors;

	for (AActor* Actor : ActorsToReveal)
	{
		if (IsValid(Actor))
		{
			Actor->SetActorHiddenInGame(false);
		}
	}
}

void APlayerCharacter::OnVisionReleased()
{
	if (!bVisionEnabled)
	{
		return;
	}

	for (AActor* Actor : VisionActors)
	{
		if (IsValid(Actor))
		{
			Actor->SetActorHiddenInGame(true);
		}
	}

	for (AActor* Actor : FalseVisionActors)
	{
		if (IsValid(Actor))
		{
			Actor->SetActorHiddenInGame(true);
		}
	}
}

void APlayerCharacter::AddVisionActor(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	VisionActors.AddUnique(Actor);

	if (!bVisionEnabled || bVisionCurse)
	{
		Actor->SetActorHiddenInGame(true);
	}
}

void APlayerCharacter::AddFalseVisionActor(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	FalseVisionActors.AddUnique(Actor);

	if (!bVisionEnabled || !bVisionCurse)
	{
		Actor->SetActorHiddenInGame(true);
	}
}

void APlayerCharacter::RemoveVisionActor(AActor* Actor)
{
	VisionActors.Remove(Actor);

	if (IsValid(Actor))
	{
		Actor->SetActorHiddenInGame(false);
	}
}

void APlayerCharacter::RemoveFalseVisionActor(AActor* Actor)
{
	FalseVisionActors.Remove(Actor);

	if (IsValid(Actor))
	{
		Actor->SetActorHiddenInGame(false);
	}
}

void APlayerCharacter::ClearVisionActors()
{
	for (AActor* Actor : VisionActors)
	{
		if (IsValid(Actor))
		{
			Actor->SetActorHiddenInGame(false);
		}
	}

	VisionActors.Empty();
}

void APlayerCharacter::ClearFalseVisionActors()
{
	for (AActor* Actor : FalseVisionActors)
	{
		if (IsValid(Actor))
		{
			Actor->SetActorHiddenInGame(false);
		}
	}

	FalseVisionActors.Empty();
}

void APlayerCharacter::OnGlidePressed()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	if (!bGlideEnabled || !MovementComponent || !MovementComponent->IsFalling())
	{
		return;
	}

	bGliding = true;

	MovementComponent->GravityScale = GlideGravityScale;
	MovementComponent->AirControl = bGlideCurse ? CursedGlideAirControl : GlideAirControl;

	FVector Velocity = MovementComponent->Velocity;
	const float MaxFallSpeed = bGlideCurse ? CursedGlideFallSpeed : GlideFallSpeed;

	if (Velocity.Z < -MaxFallSpeed)
	{
		Velocity.Z = -MaxFallSpeed;
		MovementComponent->Velocity = Velocity;
	}
}

void APlayerCharacter::OnGlideReleased()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	if (!bGliding || !MovementComponent)
	{
		return;
	}

	bGliding = false;

	MovementComponent->GravityScale = DefaultGravityScale;
	MovementComponent->AirControl = bDoubleJumpCurse ? CursedGlideAirControl : DefaultAirControl;
}

void APlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	AirJumpCount = 0;
	bJumpHeld = false;
	JumpHoldTime = 0.f;
	bGliding = false;

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	if (MovementComponent)
	{
		MovementComponent->GravityScale = DefaultGravityScale;
		MovementComponent->AirControl = bDoubleJumpCurse ? CursedGlideAirControl : DefaultAirControl;

		if (!bDashIceActive)
		{
			MovementComponent->GroundFriction = DefaultGroundFriction;
			MovementComponent->BrakingDecelerationWalking = DefaultBrakingDecelerationWalking;
		}
	}

	if (bHighJumpFallFOVActive)
	{
		SetNormalFOV();
		bHighJumpFallFOVActive = false;
	}
}

void APlayerCharacter::SetNormalFOV()
{
	if (!CameraActor)
	{
		return;
	}

	if (MoveSpeed > 600.f)
	{
		CameraActor->TargeFieldOfView = 120.f;
	}
	else
	{
		CameraActor->TargeFieldOfView = 100.f;
	}
}

void APlayerCharacter::SetHighJumpCurseFOV()
{
	if (!CameraActor)
	{
		return;
	}

	CameraActor->TargeFieldOfView = HighJumpCurseFOV;
}

void APlayerCharacter::OnPausePressed()
{
}

void APlayerCharacter::OnRestartPressed()
{
	SetActorLocation(RestartLocation);
}

void APlayerCharacter::SetRestartLocation(const FVector& Location)
{
	RestartLocation = Location;
}

void APlayerCharacter::SetAbilityEnabled(EPlayerAbility Ability, bool bEnabled)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	switch (Ability)
	{
	case EPlayerAbility::Dash:
		bDashEnabled = bEnabled;

		if (!bEnabled && bDashIceActive)
		{
			bDashIceActive = false;

			if (MovementComponent)
			{
				MovementComponent->GroundFriction = DefaultGroundFriction;
				MovementComponent->BrakingDecelerationWalking = DefaultBrakingDecelerationWalking;
			}
		}
		else if (bEnabled && bDashCurse && MovementComponent)
		{
			bDashIceActive = true;
			MovementComponent->GroundFriction = DashIceGroundFriction;
			MovementComponent->BrakingDecelerationWalking = DashIceBrakingDeceleration;
		}
		break;

	case EPlayerAbility::HighJump:
		bHighJumpEnabled = bEnabled;

		if (!bEnabled && bHighJumpFallFOVActive)
		{
			SetNormalFOV();
			bHighJumpFallFOVActive = false;
		}
		break;

	case EPlayerAbility::DoubleJump:
		bDoubleJumpEnabled = bEnabled;
		break;

	case EPlayerAbility::LedgeGrab:
		bLedgeGrabEnabled = bEnabled;

		if (!bEnabled && bGrabbingLedge)
		{
			OnGrabReleased();
		}
		break;

	case EPlayerAbility::Vision:
		bVisionEnabled = bEnabled;

		if (!bEnabled)
		{
			OnVisionReleased();
		}
		break;

	case EPlayerAbility::Glide:
		bGlideEnabled = bEnabled;

		if (!bEnabled && bGliding)
		{
			OnGlideReleased();
		}
		break;
	}
	if (AbilityListWidget)
	{
		AbilityListWidget->RefreshList();
	}
}

void APlayerCharacter::SetCurseEnabled(EPlayerCurse Curse, bool bEnabled)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	switch (Curse)
	{
	case EPlayerCurse::Dash:
		bDashCurse = bEnabled;

		if (MovementComponent && bDashEnabled)
		{
			bDashIceActive = bEnabled;

			MovementComponent->GroundFriction = bEnabled
				? DashIceGroundFriction
				: DefaultGroundFriction;

			MovementComponent->BrakingDecelerationWalking = bEnabled
				? DashIceBrakingDeceleration
				: DefaultBrakingDecelerationWalking;
		}
		break;

	case EPlayerCurse::HighJump:
		bHighJumpCurse = bEnabled;

		if (!bEnabled && bHighJumpFallFOVActive)
		{
			SetNormalFOV();
			bHighJumpFallFOVActive = false;
		}
		break;

	case EPlayerCurse::DoubleJump:
		bDoubleJumpCurse = bEnabled;

		if (MovementComponent && !bJumpHeld)
		{
			MovementComponent->AirControl = bGliding && bGlideCurse
				? CursedGlideAirControl
				: bGliding
				? GlideAirControl
				: DefaultAirControl;
		}
		break;

	case EPlayerCurse::LedgeGrab:
		bLedgeGrabCurse = bEnabled;
		break;

	case EPlayerCurse::Vision:
		bVisionCurse = bEnabled;

		if (bVisionEnabled)
		{
			OnVisionReleased();
			OnVisionPressed();
		}
		break;

	case EPlayerCurse::Glide:
		bGlideCurse = bEnabled;

		if (bGliding && MovementComponent)
		{
			MovementComponent->AirControl = bEnabled
				? CursedGlideAirControl
				: GlideAirControl;

			FVector Velocity = MovementComponent->Velocity;
			const float MaxFallSpeed = bEnabled
				? CursedGlideFallSpeed
				: GlideFallSpeed;

			if (Velocity.Z < -MaxFallSpeed)
			{
				Velocity.Z = -MaxFallSpeed;
				MovementComponent->Velocity = Velocity;
			}
		}
		break;
	}
	if (AbilityListWidget)
	{
		AbilityListWidget->RefreshList();
	}
}
