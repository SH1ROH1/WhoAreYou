#pragma once

#include "CoreMinimal.h"
#include "TemplateCharacter.h"


#include "PlayerCharacter.generated.h"



UCLASS()
class WHOAREYOU_API APlayerCharacter : public ATemplateCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();


protected:
	virtual void BeginPlay() override;

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

	void OnDashPressed();

	void OnGrabPressed();
	void OnGrabReleased();

	void OnVisionPressed();

	void OnPausePressed();

	void OnRestartPressed();


public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
};