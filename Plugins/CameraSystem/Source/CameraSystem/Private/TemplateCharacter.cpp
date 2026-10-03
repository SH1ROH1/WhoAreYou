#include "TemplateCharacter.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "GameCamera.h"
#include "MovementEventer.h"

ATemplateCharacter::ATemplateCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATemplateCharacter::BeginPlay()
{
	Super::BeginPlay();

	bUseControllerRotationYaw = false;

	if (!CameraClass)
	{
		UE_LOG(LogTemp, Error, TEXT("CameraClass == nullptr"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("CameraClass = %s"), *GetNameSafe(CameraClass));

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CameraActor = GetWorld()->SpawnActor<AGameCamera>(CameraClass, GetActorLocation(), FRotator::ZeroRotator, Params);

	if (!CameraActor)
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED TO SPAWN CAMERA"));
		return;
	}

	CameraActor->SetPlayerTarget(this);

	UE_LOG(LogTemp, Warning, TEXT("Camera spawned: %s"), *GetNameSafe(CameraActor));

	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerController == nullptr"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("PlayerController: %s"), *GetNameSafe(PC));

	PC->SetViewTargetWithBlend(CameraActor, 0.2f);

	UE_LOG(LogTemp, Warning, TEXT("ViewTarget = %s"), *GetNameSafe(PC->GetViewTarget()));
}

void ATemplateCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ApplyMovement(DeltaTime);
	UpdateCameraRotation();
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void ATemplateCharacter::Move_X(float Value)
{
	MoveX = Value;
}

void ATemplateCharacter::Move_Z(float Value)
{
	MoveZ = Value;
}

void ATemplateCharacter::ApplyMovement(float DeltaTime)
{
	switch (MovementMode)
	{
	case EPlayerMovementMode::RotateToMovement:
	{
		const float ForwardInput = MoveX;
		const float TurnInput = MoveZ;

		if (!FMath::IsNearlyZero(TurnInput))
		{
			const float TurnAmount = TurnInput * RotationSpeed * DeltaTime * 100.f;

			FRotator Rotation = GetActorRotation();
			Rotation.Yaw += TurnAmount;

			SetActorRotation(FRotator(0.f, Rotation.Yaw, 0.f));
		}

		if (!FMath::IsNearlyZero(ForwardInput))
		{
			const FVector Forward = GetActorForwardVector();

			AddMovementInput(Forward, ForwardInput);
		}

		break;
	}

	case EPlayerMovementMode::TwoAxis:
	{
		const FVector Movement(MoveX, MoveZ, 0.f);

		if (!Movement.IsNearlyZero())
		{
			AddMovementInput(Movement.GetSafeNormal2D(), Movement.Size());
		}

		break;
	}

	case EPlayerMovementMode::MoveToCamera:
	{
		FVector Movement = FVector::ZeroVector;

		if (CameraActor)
		{
			FVector Forward = CameraActor->GetActorForwardVector();
			FVector Right = CameraActor->GetActorRightVector();

			Forward.Z = 0.f;
			Right.Z = 0.f;

			Forward.Normalize();
			Right.Normalize();

			Movement = Forward * MoveX + Right * MoveZ;
		}
		else
		{
			Movement = FVector(MoveX, MoveZ, 0.f);
		}

		Movement.Z = 0.f;

		if (!Movement.IsNearlyZero())
		{
			Movement.Normalize();

			AddMovementInput(Movement, 1.f);

			if (bRotateToMovement)
			{
				const FRotator TargetRotation = Movement.Rotation();
				const FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, RotationSpeed);

				SetActorRotation(FRotator(0.f, NewRotation.Yaw, 0.f));
			}
		}

		break;
	}
	}
}

void ATemplateCharacter::Look_X(float Value)
{
	LookX = Value;
}

void ATemplateCharacter::Look_Y(float Value)
{
	LookY = Value;
}

void ATemplateCharacter::UpdateCameraRotation()
{
	if (!CameraActor)
	{
		return;
	}

	AGameCamera* Camera = CameraActor;

	if (Camera->GetState().ControlMode != ECameraControlMode::FirstPerson)
	{
		return;
	}

	FGameCameraState State = Camera->GetState();

	State.OrbitYaw += LookX * State.MouseSensitivity;
	State.OrbitPitch += LookY * State.MouseSensitivity;

	State.OrbitPitch = FMath::Clamp(State.OrbitPitch, State.MinPitch, State.MaxPitch);

	Camera->SetCameraState(State, ECameraTransitionMode::Instant);

	LookX = 0.f;
	LookY = 0.f;
}

void ATemplateCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveZ", this, &ATemplateCharacter::Move_Z);
	PlayerInputComponent->BindAxis("MoveX", this, &ATemplateCharacter::Move_X);

	PlayerInputComponent->BindAxis("LookX", this, &ATemplateCharacter::Look_X);
	PlayerInputComponent->BindAxis("LookY", this, &ATemplateCharacter::Look_Y);
}

void ATemplateCharacter::SetLastTriggeredEventer(AMovementEventer* Eventer)
{
	LastTriggeredEventer = Eventer;
}

void ATemplateCharacter::ClearLastTriggeredEventer(AMovementEventer* Eventer)
{
	if (LastTriggeredEventer.Get() == Eventer)
	{
		LastTriggeredEventer = nullptr;
	}
}

bool ATemplateCharacter::IsLastTriggeredEventer(AMovementEventer* Eventer) const
{
	return LastTriggeredEventer.Get() == Eventer;
}


AGameCamera* ATemplateCharacter::GetCameraActor() const
{
	return CameraActor;
}
void ATemplateCharacter::ChangeMovementMode(EPlayerMovementMode NewMode)
{
	MovementMode = NewMode;
}
EPlayerMovementMode ATemplateCharacter::GetMovementMode() const
{
	return MovementMode;
}