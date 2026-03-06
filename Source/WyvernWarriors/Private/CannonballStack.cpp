#include "CannonballStack.h"
#include "Components/SphereComponent.h"

// Sets default values
ACannonballStack::ACannonballStack()
{
	PrimaryActorTick.bCanEverTick = false; // Does not need to stack
	
	// Create sphere component
	SphereComponent = CreateDefaultSubobject<USphereComponent>("SphereComponent");
	SetRootComponent(SphereComponent);
}

// Sets cannonball stack properly in level
void ACannonballStack::SetActiveness(bool const bIsActive) const
{
	SphereComponent->SetVisibility(bIsActive); // Set visibility
	SetPickupCollision(bIsActive); // Set collision usage
	
	// Move to proper location
	if (bIsActive)
	{
		SphereComponent->AddWorldOffset(FVector(0.0f, 0.0f, 10000.0f));
	}
	// Move beneath level
	else
	{
		SphereComponent->AddWorldOffset(FVector(0.0f, 0.0f, -10000.0f));
	}
}

// Sets cannonball stack collision
void ACannonballStack::SetPickupCollision(bool const bHasCollision) const
{
	// Set collision enabled
	if (bHasCollision)
	{	
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	// Set collision disabled1
	else
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}
