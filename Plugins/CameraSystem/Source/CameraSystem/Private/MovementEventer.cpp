#include "MovementEventer.h"

#include "EngineUtils.h"
#include "GameFramework/Character.h"

AMovementEventer::AMovementEventer()
{
	PrimaryActorTick.bCanEverTick = true;

	OnActorBeginOverlap.AddDynamic(this, &AMovementEventer::OnActorEntered);
	OnActorEndOverlap.AddDynamic(this, &AMovementEventer::OnActorLeft);
}

void AMovementEventer::BeginPlay()
{
	Super::BeginPlay();

}

void AMovementEventer::OnActorEntered(AActor* OverlappedActor, AActor* OtherActor)
{
	if (bTriggered)
	{
		return;
	}

	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	if (!OtherActor->IsA<ATemplateCharacter>())
	{
		return;
	}

	DemoCharacter = Cast<ATemplateCharacter>(OtherActor);

	if (!DemoCharacter)
	{
		return;
	}

	if (DemoCharacter->IsLastTriggeredEventer(this))
	{
		return;
	}

	if (DemoCharacter->GetMovementMode() == MovementMode)
	{
		bTriggered = true;
		DemoCharacter->SetLastTriggeredEventer(this);
		return;
	}

	DemoCharacter->ChangeMovementMode(MovementMode);

	bTriggered = true;
	DemoCharacter->SetLastTriggeredEventer(this);
}

void AMovementEventer::OnActorLeft(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	if (!OtherActor->IsA<ACharacter>())
	{
		return;
	}

	if (!bTriggered)
	{
		return;
	}

	DemoCharacter = Cast<ATemplateCharacter>(OtherActor);

	if (!DemoCharacter)
	{
		return;
	}

	if (!DemoCharacter->IsLastTriggeredEventer(this))
	{
		return;
	}

	bTriggered = false;
	DemoCharacter->ClearLastTriggeredEventer(this);

	if (!bReturnMovementmode)
	{
		return;
	}

	if (DemoCharacter->GetMovementMode() == ReturnMovementMode)
	{
		return;
	}

	DemoCharacter->ChangeMovementMode(ReturnMovementMode);
}