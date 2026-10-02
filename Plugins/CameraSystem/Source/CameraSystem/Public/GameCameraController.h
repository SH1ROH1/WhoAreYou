#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameCamera.h"
#include "GameCameraPresetData.h"
#include "GameCameraController.generated.h"

class USplineComponent;
class AGameCameraEventer;

UCLASS(Blueprintable)
class CAMERASYSTEM_API AGameCameraController : public AActor
{
	GENERATED_BODY()

public:
	AGameCameraController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:

	UPROPERTY()
	TArray<TObjectPtr<AGameCamera>> CachedCameras;

	UPROPERTY()
	TObjectPtr<USplineComponent> ActiveSpline = nullptr;

	bool bTriggered = false;
	bool bPathActive = false;
	bool bPathFinished = false;

	float CurrentPathDistance = 0.f;

	float ActiveStartDistanceThreshold = 0.f;
	float ActivePathOffset = 0.f;
	float ActiveFollowSmoothing = 5.f;

	bool bActivePathLoop = false;

public:

	//=====================================================
	// Camera Target
	//=====================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	TObjectPtr<AGameCamera> GameCamera = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FName CameraTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bUseCameraTag = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bAffectAllCameras = false;


	//=====================================================
	// State | Presets
	//=====================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Presets")
	FName PresetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Presets")
	TObjectPtr<UGameCameraPresetData> PresetData = nullptr;


	//=====================================================
	// State | Camera
	//=====================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Camera")
	FGameCameraState CameraState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Camera")
	TObjectPtr<AActor> ConnectTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Camera")
	ECameraTransitionMode TransitionMode = ECameraTransitionMode::Smooth;


	//=====================================================
	// State | Path
	//=====================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Path")
	TObjectPtr<AActor> CameraPathActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Path", meta = (ClampMin = "0.0"))
	float StartDistanceThreshold = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Path")
	float PathOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Path", meta = (ClampMin = "0.1"))
	float FollowSmoothing = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Path")
	bool bPathLoop = false;


	//=====================================================
	// State | Restore
	//=====================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Restore")
	bool bRestoreOnExit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Restore", meta = (EditCondition = "bRestoreOnExit"))
	FGameCameraState RestoreCameraState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Restore", meta = (EditCondition = "bRestoreOnExit"))
	FName RestorePresetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Restore", meta = (EditCondition = "bRestoreOnExit"))
	TObjectPtr<AActor> RestoreConnectTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Restore", meta = (EditCondition = "bRestoreOnExit"))
	ECameraTransitionMode RestoreTransitionMode = ECameraTransitionMode::Smooth;


	//=====================================================
	// State | Restore Path
	//=====================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Restore Path", meta = (EditCondition = "bRestoreOnExit"))
	TObjectPtr<AActor> RestoreCameraPathActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Restore Path", meta = (EditCondition = "bRestoreOnExit", ClampMin = "0.0"))
	float RestoreStartDistanceThreshold = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Restore Path", meta = (EditCondition = "bRestoreOnExit"))
	float RestorePathOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Restore Path", meta = (EditCondition = "bRestoreOnExit", ClampMin = "0.1"))
	float RestoreFollowSmoothing = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Restore Path", meta = (EditCondition = "bRestoreOnExit"))
	bool bRestorePathLoop = false;


	//=====================================================
	// Trigger
	//=====================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Trigger")
	bool bFireOnlyOnce = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State | Trigger", meta = (ClampMin = "0.0"))
	float ActivationDelay = 0.f;

	UPROPERTY(Transient)
	float ActivationTimer = 0.f;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> PendingActor;


	//=====================================================
	// Camera Search
	//=====================================================

	UFUNCTION(BlueprintCallable, Category = "Camera Search")
	void FindAllCameras();

	UFUNCTION(BlueprintCallable, Category = "Camera Search")
	AGameCamera* FindCameraByTag(FName Tag);

	UFUNCTION(BlueprintPure, Category = "Camera Search")
	AGameCamera* GetTargetCamera();


	//=====================================================
	// Camera
	//=====================================================

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCamera(AGameCamera* NewCamera);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraTargetByTag(FName NewCameraTag);

	UFUNCTION(BlueprintPure, Category = "Camera")
	FName GetCameraTag() const;


	//=====================================================
	// Camera State
	//=====================================================

	UFUNCTION(BlueprintCallable, Category = "Camera State")
	void ApplyCameraState(const FGameCameraState& NewState);

	UFUNCTION(BlueprintCallable, Category = "Camera State")
	void SetCameraState(const FGameCameraState& NewState);

	UFUNCTION(BlueprintPure, Category = "Camera State")
	FGameCameraState GetCameraState() const;

	UFUNCTION(BlueprintCallable, Category = "Camera State")
	void GetCameraStateFromCamera(
		AGameCamera* TargetCamera,
		FGameCameraState& OutState
	) const;


	//=====================================================
	// Camera Command
	//=====================================================

	UFUNCTION(BlueprintCallable, Category = "Camera Command")
	void ApplyCameraCommand();

	UFUNCTION(BlueprintCallable, Category = "Camera Command")
	void ApplyCameraCommandToCamera(
		AGameCamera* TargetCamera
	);

	UFUNCTION(BlueprintCallable, Category = "Camera Command")
	void ApplyParametersToAllCameras();


	//=====================================================
	// Position
	//=====================================================

	UFUNCTION(BlueprintCallable, Category = "Camera | Position")
	void SetCameraPosition(FVector NewPosition);

	UFUNCTION(BlueprintCallable, Category = "Camera | Position")
	void SetCameraPositionMode(
		ECameraPositionMode NewMode
	);

	UFUNCTION(BlueprintCallable, Category = "Camera | Position")
	void SetCameraTransform(
		FVector NewPosition,
		FRotator NewRotation,
		ECameraPositionMode NewPositionMode
	);


	//=====================================================
	// Rotation
	//=====================================================

	UFUNCTION(BlueprintCallable, Category = "Camera | Rotation")
	void SetCameraRotation(FRotator NewRotation);

	UFUNCTION(BlueprintCallable, Category = "Camera | Rotation")
	void SetPathRotation(FRotator NewRotation);

	UFUNCTION(BlueprintCallable, Category = "Camera | Rotation")
	void ClearPathRotation();


	//=====================================================
	// Interactive Camera
	//=====================================================

	UFUNCTION(BlueprintCallable, Category = "Camera | Interactive")
	void SetControlMode(
		ECameraControlMode NewControlMode
	);

	UFUNCTION(BlueprintCallable, Category = "Camera | Interactive")
	void SetLookMode(
		ECameraLookMode NewLookMode
	);

	UFUNCTION(BlueprintCallable, Category = "Camera | Interactive")
	void SetMouseSensitivity(
		float NewSensitivity
	);

	UFUNCTION(BlueprintCallable, Category = "Camera | Interactive")
	void SetOrbitYaw(
		float NewYaw
	);

	UFUNCTION(BlueprintCallable, Category = "Camera | Interactive")
	void SetOrbitPitch(
		float NewPitch
	);

	UFUNCTION(BlueprintCallable, Category = "Camera | Interactive")
	void SetOrbitDistance(
		float NewDistance
	);

	UFUNCTION(BlueprintCallable, Category = "Camera | Interactive")
	void SetAimDistance(
		float NewDistance
	);

	UFUNCTION(BlueprintCallable, Category = "Camera | Interactive")
	void SetAimHeight(
		float NewHeight
	);


	//=====================================================
	// Target
	//=====================================================

	UFUNCTION(BlueprintCallable, Category = "Target")
	void SetConnectTarget(
		AActor* NewConnectTarget
	);

	UFUNCTION(BlueprintPure, Category = "Target")
	AActor* GetConnectTarget() const;


	//=====================================================
	// Presets
	//=====================================================

	UFUNCTION(BlueprintCallable, Category = "Presets")
	void ApplyPreset(
		FName NewPresetName
	);


	//=====================================================
	// Restore
	//=====================================================

	UFUNCTION(BlueprintCallable, Category = "Restore")
	void ApplyRestore();

	UFUNCTION(BlueprintCallable, Category = "Restore")
	void ResetCamera();


	//=====================================================
	// Trigger / Eventer
	//=====================================================

	UFUNCTION(BlueprintCallable, Category = "Trigger")
	void ActivateEventer(
		AActor* OtherActor
	);

	UFUNCTION(BlueprintCallable, Category = "Trigger")
	void RestoreEventer(
		AActor* OtherActor
	);


	//=====================================================
	// Path
	//=====================================================

	UFUNCTION(BlueprintCallable, Category = "Path")
	void StartPath();

	UFUNCTION(BlueprintCallable, Category = "Path")
	void StopPath();

	UFUNCTION(BlueprintPure, Category = "Path")
	bool IsPathActive() const;


	//=====================================================
	// Restore Path
	//=====================================================

	UFUNCTION(BlueprintCallable, Category = "Restore Path")
	void StartRestorePath();

	UFUNCTION(BlueprintCallable, Category = "Restore Path")
	void StopRestorePath();


	//=====================================================
	// Preset Data
	//=====================================================

	UFUNCTION(BlueprintCallable, Category = "Preset Data")
	void SavePreset();

	UFUNCTION(BlueprintCallable, Category = "Preset Data")
	void LoadPreset();

	UFUNCTION(BlueprintCallable, Category = "Preset Data")
	void DeletePreset();


private:

	AGameCamera* ResolveTargetCamera() const;

	void ApplyToCamera(
		AGameCamera* TargetCamera,
		const FGameCameraState& State,
		AActor* NewConnectTarget,
		ECameraTransitionMode NewTransitionMode
	);

	void UpdatePath(float DeltaTime);

	bool FindClosestPathDistance(
		float& OutDistance
	) const;

	void ApplyTargets(
		AActor* NewConnectTarget
	);

	void ApplyPathRotation();

	void CheckActivationDelay(
		float DeltaTime
	);
};