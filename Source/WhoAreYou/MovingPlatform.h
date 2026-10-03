#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MovingPlatform.generated.h"


class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class WHOAREYOU_API AMovingPlatform : public AActor
{
	GENERATED_BODY()

public:
	AMovingPlatform();

	USceneComponent* Root;

	UPROPERTY(EditAnywhere, Category = "Moving Platform")
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Category = "Moving Platform")
	FVector FirstLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Moving Platform")
	FVector SecondLocation = FVector::ZeroVector;


protected:
	virtual void BeginPlay() override;

	FTimerHandle Timer;



	FVector CurrentLocation = FVector::ZeroVector;

	bool bSwitchNext = false;

	void MovePlatform();

	float TimeRate = 0.016f;
	float InterpSpeed = 0.5f;
};
