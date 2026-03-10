#include "CannonballStack.h"

// Sets default values
ACannonballStack::ACannonballStack()
{
	PrimaryActorTick.bCanEverTick = false; // Does not need to stack
}

// Sets cannonball stack properly in level
void ACannonballStack::SetActiveness(bool const bIsActive)
{
	SetActorHiddenInGame(!bIsActive); // Set visibility
	SetActorEnableCollision(bIsActive);  // Set collision usage
}
