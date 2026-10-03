#include "MovingPlatform.h"

#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"

AMovingPlatform::AMovingPlatform()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);

}

void AMovingPlatform::BeginPlay()
{
	Super::BeginPlay();

	CurrentLocation = GetActorLocation();

	GetWorldTimerManager().SetTimer(Timer, this, &AMovingPlatform::MovePlatform, TimeRate, true);
}

void AMovingPlatform::MovePlatform()
{
	if (FVector::Dist(CurrentLocation, FirstLocation) <= 10)
	{
		bSwitchNext = false;
	}
	else if (FVector::Dist(CurrentLocation, SecondLocation) <= 10)
	{
		bSwitchNext = true;
	}
	if (bSwitchNext)
	{
		CurrentLocation = FMath::VInterpTo(CurrentLocation, FirstLocation, TimeRate, InterpSpeed);
	}
	else
	{
		CurrentLocation = FMath::VInterpTo(CurrentLocation, SecondLocation, TimeRate, InterpSpeed);
	}
	SetActorLocation(CurrentLocation);
}