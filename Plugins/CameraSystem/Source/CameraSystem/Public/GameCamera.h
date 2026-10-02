#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "GameCamera.generated.h"

class AActor;
class AGameCameraEventer;

UENUM(BlueprintType)
enum class ECameraTransitionMode : uint8
{
	Smooth UMETA(DisplayName = "Smooth"),
	Instant UMETA(DisplayName = "Instant")
};

UENUM(BlueprintType)
enum class ECameraPositionMode : uint8
{
	World UMETA(DisplayName = "World"),
	ConnectTarget UMETA(DisplayName = "Connect Target"),
	PlayerTarget UMETA(DisplayName = "Player Target")
};

UENUM(BlueprintType)
enum class ECameraFollowTarget : uint8
{
	None UMETA(DisplayName = "None"),
	ConnectTarget UMETA(DisplayName = "Connect Target"),
	PlayerTarget UMETA(DisplayName = "Player Target")
};

UENUM(BlueprintType)
enum class ECameraControlMode : uint8
{
	None UMETA(DisplayName = "None"),
	FirstPerson UMETA(DisplayName = "First Person"),
	ThirdPerson UMETA(DisplayName = "Third Person")
};

UENUM(BlueprintType)
enum class ECameraLookMode : uint8
{
	LookAtTarget UMETA(DisplayName = "Look At Target"),
	LookAtAim UMETA(DisplayName = "Look At Aim")
};

USTRUCT(BlueprintType)
struct FGameCameraState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
	ECameraPositionMode PositionMode = ECameraPositionMode::PlayerTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
	FVector Position = FVector(0.f, -600.f, 600.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	ECameraFollowTarget FollowTarget = ECameraFollowTarget::PlayerTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bUseCollision = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	FRotator Rotation = FRotator(-60.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	float LockOnRotation = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	bool bUseTargetRotation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	bool bUsePathRotation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bLookAtTarget = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MovementSpeed = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float RotationSpeed = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float FieldOfView = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float BlendSpeed = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Control")
	ECameraControlMode ControlMode = ECameraControlMode::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Control")
	ECameraLookMode LookMode = ECameraLookMode::LookAtTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Control", meta = (ClampMin = "0.0"))
	float MouseSensitivity = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Control")
	float OrbitYaw = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Control", meta = (ClampMin = "-89.0", ClampMax = "89.0"))
	float OrbitPitch = -20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Control", meta = (ClampMin = "-89.0", ClampMax = "89.0"))
	float MinPitch = -80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Control", meta = (ClampMin = "-89.0", ClampMax = "89.0"))
	float MaxPitch = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Control", meta = (ClampMin = "1.0"))
	float OrbitDistance = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Control", meta = (ClampMin = "0.0"))
	float AimDistance = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Control")
	float AimHeight = 0.f;
};

USTRUCT(BlueprintType)
struct FGameCameraPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	FGameCameraState State;
};

UCLASS()
class CAMERASYSTEM_API AGameCamera : public ACameraActor
{
	GENERATED_BODY()

public:
	AGameCamera();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:

	bool bPathRotationValid = false;
	bool bInteractiveInitialized = false;

	FRotator PathRotation = FRotator::ZeroRotator;

	FVector FixedWorldPosition = FVector::ZeroVector;

	void UpdateBlend(float DeltaTime);
	void UpdatePosition(float DeltaTime);
	void UpdateRotation(float DeltaTime);
	void UpdateInteractiveCamera(float DeltaTime);

	TWeakObjectPtr<AGameCameraEventer> LastTriggeredEventer;

	FVector CalculateDesiredLocation() const;
	FVector CalculateInteractiveLocation() const;
	FVector CalculateAimPoint() const;

	FVector CalculateConnectTargetCameraLocation() const;

	AActor* GetPositionTarget() const;
	AActor* GetFollowTarget() const;
	AActor* GetRotationTarget() const;
	AActor* GetInteractiveTarget() const;

	FVector GetWorldPositionFromState(const FGameCameraState& State) const;
	FVector GetRelativePositionFromWorld(const FVector& WorldPosition, ECameraPositionMode PositionMode) const;

	void ConvertPositionSpace(ECameraPositionMode NewPositionMode);

	void InitializeInteractiveCamera();

public:

	UFUNCTION(BlueprintCallable, Category = "Trigger History")
	void SetLastTriggeredEventer(AGameCameraEventer* Eventer);

	UFUNCTION(BlueprintCallable, Category = "Trigger History")
	void ClearLastTriggeredEventer(AGameCameraEventer* Eventer);

	UFUNCTION(BlueprintPure, Category = "Trigger History")
	bool IsLastTriggeredEventer(AGameCameraEventer* Eventer) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FGameCameraState CurrentState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FGameCameraState TargetState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	TArray<FGameCameraPreset> Presets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	TObjectPtr<AActor> PlayerTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	TObjectPtr<AActor> ConnectTarget;

	UFUNCTION(BlueprintCallable, Category = "Target")
	void SetPlayerTarget(AActor* NewTarget);

	UFUNCTION(BlueprintPure, Category = "Target")
	AActor* GetPlayerTarget() const
	{
		return PlayerTarget;
	}

	UFUNCTION(BlueprintCallable, Category = "Target")
	void SetConnectTarget(AActor* NewTarget);

	UFUNCTION(BlueprintPure, Category = "Target")
	AActor* GetConnectTarget() const
	{
		return ConnectTarget;
	}

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetState(const FGameCameraState& NewState);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraState(const FGameCameraState& NewState, ECameraTransitionMode TransitionMode = ECameraTransitionMode::Smooth);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetPositionMode(ECameraPositionMode NewMode);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraTransform(FVector NewPosition, FRotator NewRotation, ECameraPositionMode NewPositionMode);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetPathRotation(FRotator NewRotation);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ClearPathRotation();

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ResetState();

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ApplyPreset(FName PresetName);

	UFUNCTION(BlueprintPure, Category = "Camera")
	FGameCameraState GetState() const
	{
		return CurrentState;
	}
};