#include "CannonManagerComponent.h"
#include "CannonballStack.h"
#include "Cannon.h"

// Set cannons as visible and able to collide
void UCannonManagerComponent::SetCannonballsActive()
{
	// Go through all cannonball stacks
	for (const ACannonballStack* CannonballStack : CannonballStacks)
	{
		if (IsValid(CannonballStack))
		{
			CannonballStack->SetActiveness(true);
		}
	}
}

// Set a random cannon as able to fire at the boss
void UCannonManagerComponent::SetCannonFireable(bool bCanFire)
{
	// No more cannons to set as able to fire
	if (InactiveCannons.IsEmpty())
	{
		return;
	}
	
	ACannon* CannonToFire; // Initialization of cannon reference
	
	// Move cannon between from inactive to active array
	if (bCanFire)
	{
	    CannonToFire = InactiveCannons[FMath::RandRange(0, InactiveCannons.Num() - 1)]; // Get random cannon from array
		InactiveCannons.Remove(CannonToFire); // Remove cannon from inactive cannons array
		ActiveCannons.Add(CannonToFire); // Add cannon to active cannons array
	}
	// Move cannon between from active to inactive array
	else
	{
	    CannonToFire = ActiveCannons[FMath::RandRange(0, ActiveCannons.Num() - 1)]; // Get random cannon from array
		ActiveCannons.Remove(CannonToFire); // Remove cannon from active cannons array
		InactiveCannons.Add(CannonToFire); // Add cannon to inactive cannons array
	}
	
	CannonToFire->SetFirable(bCanFire);
}

// Sets multiple random cannons as able to fire at the boss
void UCannonManagerComponent::SetMultipleCannonsFireable(bool bCanFire, int32 NumberOfCannons)
{
	// Set multiple cannons' ability to fire
	for (int i = 0; i < NumberOfCannons; i++)
	{
		SetCannonFireable(bCanFire);
	}
}

