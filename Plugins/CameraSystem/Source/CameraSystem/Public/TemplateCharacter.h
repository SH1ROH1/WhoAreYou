#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TemplateCharacter.generated.h"

class AGameCamera;
class AMovementEventer;

UENUM(BlueprintType)
enum class EPlayerMovementMode : uint8
{
	RotateToMovement UMETA(DisplayName = "Rotate To Movement"),
	TwoAxis UMETA(DisplayName = "Two Axis"),
	MoveToCamera UMETA(DisplayName = "Move To Camera")
};

UCLASS()
class CAMERASYSTEM_API ATemplateCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATemplateCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	TSubclassOf<AGameCamera> CameraClass;

	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	TObjectPtr<AGameCamera> CameraActor;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MoveSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RotationSpeed = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bRotateToMovement = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MouseSensitivity = 2.f;

	UFUNCTION(BlueprintPure, Category = "Camera")
	AGameCamera* GetCameraActor() const;

	UFUNCTION(BlueprintCallable, Category = "Character")
	void ChangeMovementMode(EPlayerMovementMode NewMode);

	UFUNCTION(BlueprintPure, Category = "Character")
	EPlayerMovementMode GetMovementMode() const;

	TWeakObjectPtr<AMovementEventer> LastTriggeredEventer;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	EPlayerMovementMode MovementMode = EPlayerMovementMode::MoveToCamera;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Movement input

	float MoveX = 0.f;
	float MoveZ = 0.f;

	void Move_X(float Value);
	void Move_Z(float Value);

	void ApplyMovement(float DeltaTime);

	// Mouse input

	float LookX = 0.f;
	float LookY = 0.f;

	void Look_X(float Value);
	void Look_Y(float Value);

	void UpdateCameraRotation();

public:

	UFUNCTION(BlueprintCallable, Category = "Trigger History")
	void SetLastTriggeredEventer(AMovementEventer* Eventer);

	UFUNCTION(BlueprintCallable, Category = "Trigger History")
	void ClearLastTriggeredEventer(AMovementEventer* Eventer);

	UFUNCTION(BlueprintPure, Category = "Trigger History")
	bool IsLastTriggeredEventer(AMovementEventer* Eventer) const;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
};