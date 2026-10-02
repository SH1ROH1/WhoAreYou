#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "TemplateCharacter.h"
#include "GameCameraPresetData.h"
#include "MovementEventer.generated.h"



UCLASS(Blueprintable)
class CAMERASYSTEM_API AMovementEventer : public ATriggerVolume
{
	GENERATED_BODY()

public:
	AMovementEventer();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	EPlayerMovementMode MovementMode = EPlayerMovementMode::MoveToCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bReturnMovementmode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta= (EditCondition = "bReturnMovementmode"))
	EPlayerMovementMode ReturnMovementMode = EPlayerMovementMode::MoveToCamera;

protected:
	virtual void BeginPlay() override;


	UFUNCTION()
	void OnActorEntered(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void OnActorLeft(AActor* OverlappedActor, AActor* OtherActor);

private:


	UPROPERTY()
	ATemplateCharacter* DemoCharacter = nullptr;

	bool bTriggered = false;

};