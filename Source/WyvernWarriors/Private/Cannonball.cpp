#include "Cannonball.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameManagers/GameModeLevel.h"
#include "Components/SphereComponent.h"

// Sets default values for this actor's properties
ACannonball::ACannonball()
{
	PrimaryActorTick.bCanEverTick = false; // Does not need to tick
	
	// Set sphere component as root for collision
	SphereComponent = CreateDefaultSubobject<USphereComponent>("SphereComponent");
	SetRootComponent(SphereComponent);
	
	// Attach static mesh to root component
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMesh->SetupAttachment(RootComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement"); // Create projectile movement component
	ProjectileMovement->bAutoActivate = false; // Do not activate movement at start of level
}

// Set initial activeness and get event bus
void ACannonball::BeginPlay()
{
	Super::BeginPlay();
	InitialLocation = GetActorLocation(); // Store initial location for resetting later

	// Get game mode
	const AGameModeLevel* GameModeLevel = Cast<AGameModeLevel>(GetWorld()->GetAuthGameMode());
	if (!IsValid(GameModeLevel))
	{
		return;
	}

	EventBus = GameModeLevel->GetEventBusComponent(); // Get and assign event bus
}

// Return's the speed of the projectile
float ACannonball::GetProjectileSpeed() const
{
	return ProjectileMovement->GetMaxSpeed();
}

// Sets cannonball properly in level
void ACannonball::SetActiveness(bool const bIsActive)
{
	SetActorHiddenInGame(!bIsActive); // Set visibility
	SetActorEnableCollision(bIsActive);  // Set collision usage
	
	// Stop movement if deactivating while moving
	if (!bIsActive && ProjectileMovement->IsActive())
	{
		ProjectileMovement->StopMovementImmediately();
	}
}

// Resets the cannonball to its initial location and stops its movement
void ACannonball::ResetCannonball()
{
	ProjectileMovement->StopMovementImmediately(); // Stop movement of the cannonball
	SetActorLocation(InitialLocation); // Reset location to initial location
}
