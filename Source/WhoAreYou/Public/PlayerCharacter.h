
#pragma once

#include "CoreMinimal.h"
#include "TemplateCharacter.h"
#include "Sound/SoundBase.h"

#include "PlayerCharacter.generated.h"

class UAbilityListWidget;

UENUM(BlueprintType)
enum class EPlayerAbility : uint8
{
	Dash,
	HighJump,
	DoubleJump,
	LedgeGrab,
	Vision,
	Glide
};

UENUM(BlueprintType)
enum class EPlayerCurse : uint8
{
	Dash,
	HighJump,
	DoubleJump,
	LedgeGrab,
	Vision,
	Glide
};

UCLASS()
class WHOAREYOU_API APlayerCharacter : public ATemplateCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void SetAbilityEnabled(EPlayerAbility Ability, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Abilities|Curses")
	void SetCurseEnabled(EPlayerCurse Curse, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Restart")
	void SetRestartLocation(const FVector& Location);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
	TObjectPtr<USoundBase> InteractSound;

	UFUNCTION(BlueprintPure, Category = "Abilities")
	bool IsAbilityEnabled(EPlayerAbility Ability) const;

	UFUNCTION(BlueprintPure, Category = "Abilities|Curses")
	bool IsCurseEnabled(EPlayerCurse Curse) const;


protected:
	virtual void BeginPlay() override;




	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	bool bShowAbilityList = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (EditCondition = "bShowAbilityList"))
	TSubclassOf<UAbilityListWidget> AbilityListWidgetClass;

	UPROPERTY()
	TObjectPtr<UAbilityListWidget> AbilityListWidget;



	FVector RestartLocation = FVector::ZeroVector;

	void OnInteractPressed();
	float InteractTraceDistance = 300.f;
	float InteractCooldown = 0.5f;
	float CuurentInteractCooldown = 0.f;

	void OnSprintPressed();
	void OnSprintReleased();

	void OnCrouchPressed();
	void OnCrouchReleased();

	void OnJumpPressed();
	void OnJumpReleased();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Jump")
	float JumpVelocity = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Jump")
	float JumpHoldForce = 1800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Jump")
	float MaxJumpHoldTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Jump")
	float DoubleJumpVelocity = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Double Jump|Curse")
	float CursedDoubleJumpAirControl = 0.15f;

	bool bJumpHeld = false;
	float JumpHoldTime = 0.f;

	void OnDashPressed();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Dash")
	float DashStrength = 1600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Dash")
	float DashCooldown = 1.f;

	float CurrentDashCooldown = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Dash|Curse")
	float DashIceGroundFriction = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Dash|Curse")
	float DashIceBrakingDeceleration = 0.f;

	bool bDashIceActive = false;

	void OnGrabPressed();
	void OnGrabReleased();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Grab")
	float GrabDistance = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Grab")
	float GrabHeight = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Grab")
	float GrabWallOffset = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Grab")
	float GrabMoveSpeed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Grab")
	float GrabPushOffStrength = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Grab")
	float GrabPushOffUpward = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Grab")
	float GrabWallCheckDistance = 70.f;


	FVector GrabWallNormal = FVector::ZeroVector;
	FVector GrabWallPoint = FVector::ZeroVector;
	FRotator GrabStartRotation;

	void OnVisionPressed();
	void OnVisionReleased();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Vision")
	float VisionRadius = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Vision")
	TArray<AActor*> VisionActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Vision|Curse")
	TArray<AActor*> FalseVisionActors;

	UFUNCTION(BlueprintCallable, Category = "Abilities|Vision")
	void AddVisionActor(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Abilities|Vision")
	void AddFalseVisionActor(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Abilities|Vision")
	void RemoveVisionActor(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Abilities|Vision")
	void RemoveFalseVisionActor(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Abilities|Vision")
	void ClearVisionActors();

	UFUNCTION(BlueprintCallable, Category = "Abilities|Vision")
	void ClearFalseVisionActors();

	void OnGlidePressed();
	void OnGlideReleased();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Glide")
	float GlideGravityScale = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Glide")
	float GlideAirControl = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Glide")
	float CursedGlideAirControl = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Glide")
	float GlideFallSpeed = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Glide")
	float CursedGlideFallSpeed = 250.f;

	bool bGliding = false;
	bool bGrabbingLedge = false;

	// Abilities
	UPROPERTY(EditAnywhere, Category = "Abilities")
	bool bDashEnabled = false;

	UPROPERTY(EditAnywhere, Category = "Abilities")
	bool bHighJumpEnabled = false;

	UPROPERTY(EditAnywhere, Category = "Abilities")
	bool bDoubleJumpEnabled = false;

	UPROPERTY(EditAnywhere, Category = "Abilities")
	bool bLedgeGrabEnabled = false;

	UPROPERTY(EditAnywhere, Category = "Abilities")
	bool bVisionEnabled = false;

	UPROPERTY(EditAnywhere, Category = "Abilities")
	bool bGlideEnabled = false;

	// Curses
	UPROPERTY(EditAnywhere, Category = "Abilities|Curses")
	bool bDashCurse = false;

	UPROPERTY(EditAnywhere, Category = "Abilities|Curses")
	bool bHighJumpCurse = false;

	UPROPERTY(EditAnywhere, Category = "Abilities|Curses")
	bool bDoubleJumpCurse = false;

	UPROPERTY(EditAnywhere, Category = "Abilities|Curses")
	bool bLedgeGrabCurse = false;

	UPROPERTY(EditAnywhere, Category = "Abilities|Curses")
	bool bVisionCurse = false;

	UPROPERTY(EditAnywhere, Category = "Abilities|Curses")
	bool bGlideCurse = false;

	// High Jump
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Jump")
	float HighJumpMultiplier = 1.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Jump|Curse")
	float HighJumpCurseFOV = 160.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Jump|Curse")
	float HighJumpCurseHeight = 300.f;

	bool bHighJumpFallFOVActive = false;

	int32 AirJumpCount = 0;

	float DefaultGravityScale = 1.f;
	float DefaultAirControl = 0.35f;
	float DefaultBrakingDecelerationWalking = 2048.f;
	float DefaultGroundFriction = 8.f;
	float JumpStartHeight = 0.f;

	void SetNormalFOV();
	void SetHighJumpCurseFOV();

	void OnPausePressed();
	void OnRestartPressed();

	virtual void Landed(const FHitResult& Hit) override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
};
