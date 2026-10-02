#include "PlayerCharacter.h"

#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "GameCamera.h"

#include "CollisionQueryParams.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

// base
APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;


}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();


}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (CuurentInteractCooldown != 0.f)
	{
		CuurentInteractCooldown -= DeltaTime;
		if (CuurentInteractCooldown < 0.f)
		{
			CuurentInteractCooldown = 0.f;
		}
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerCharacter::OnInteractPressed);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APlayerCharacter::OnSprintPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayerCharacter::OnSprintReleased);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &APlayerCharacter::OnCrouchPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &APlayerCharacter::OnCrouchReleased);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::OnJumpPressed);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &APlayerCharacter::OnJumpReleased);

	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &APlayerCharacter::OnDashPressed);

	PlayerInputComponent->BindAction("Grab", IE_Pressed, this, &APlayerCharacter::OnGrabPressed);
	PlayerInputComponent->BindAction("Grab", IE_Released, this, &APlayerCharacter::OnGrabReleased);

	PlayerInputComponent->BindAction("Vision", IE_Pressed, this, &APlayerCharacter::OnVisionPressed);

	PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &APlayerCharacter::OnPausePressed);
	PlayerInputComponent->BindAction("Restart", IE_Pressed, this, &APlayerCharacter::OnRestartPressed);

}


// Input functions

// Intaract
void APlayerCharacter::OnInteractPressed()
{
	if (CuurentInteractCooldown != 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Interact is on cooldown!"));
		return;
	}
	// trace
	FVector Start = CameraActor->GetActorLocation();
	FVector Direction = CameraActor->GetActorForwardVector();
	FVector End = Start + (Direction * InteractTraceDistance);
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(CameraActor);

	// Draw debug line
	UWorld* World = GetWorld();
	if (!World) return;
	bool bHitSuccess = World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
	if (!HitResult.bBlockingHit) HitResult.ImpactPoint = End;

#if !UE_BUILD_SHIPPING
	DrawDebugLine(World, Start, End, FColor::White, false, 0.3, 0, 0.15);
	DrawDebugPoint(World, HitResult.ImpactPoint, 20.f, bHitSuccess ? FColor::Green : FColor::Red, false, 0.4f);
#endif

	// Cooldown
	CuurentInteractCooldown = InteractCooldown;

	// Success
	if (!bHitSuccess || !HitResult.GetActor()) return;

	AActor* HitActor = HitResult.GetActor();

	// Apply Damage
	//UGameplayStatics::ApplyDamage(HitActor, 1.0f, nullptr, CameraActor, UDamageType::StaticClass());
	UGameplayStatics::ApplyPointDamage(HitActor, 1.f, Direction, HitResult, GetController(), this, UDamageType::StaticClass());
}


// Sprint
void APlayerCharacter::OnSprintPressed()
{
}
void APlayerCharacter::OnSprintReleased()
{
}

// Crouch
void APlayerCharacter::OnCrouchPressed()
{
}
void APlayerCharacter::OnCrouchReleased()
{
}

// Jump
void APlayerCharacter::OnJumpPressed()
{
}
void APlayerCharacter::OnJumpReleased()
{
}


// Dash
void APlayerCharacter::OnDashPressed()
{
}

// Grab
void APlayerCharacter::OnGrabPressed()
{
}
void APlayerCharacter::OnGrabReleased()
{
}

// Vision
void APlayerCharacter::OnVisionPressed()
{
}

// Pause and Restart
void APlayerCharacter::OnPausePressed()
{
}
void APlayerCharacter::OnRestartPressed()
{
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}