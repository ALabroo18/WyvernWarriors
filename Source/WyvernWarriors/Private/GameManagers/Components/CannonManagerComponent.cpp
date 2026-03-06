#include "GameManagers/Components/CannonManagerComponent.h"
#include "BossEnemy.h"
#include "CannonballStack.h"
#include "Cannonball.h"
#include "Cannon.h"
#include "GameManagers/GameModeLevel.h"
#include "GameManagers/Components/EnemyManagerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ActivatableInterface.h"

// Set cannon activatables activeness
void UCannonManagerComponent::SetCannonActivatables(bool const bBecomeActive)
{
	// Go through all cannonballs
	for (AActor* Activatable : CannonActivatables)
	{
		if (IsValid(Activatable))
		{
			IActivatableInterface* ActivatableInterface = Cast<IActivatableInterface>(Activatable); // Cast to activatable interface
			ActivatableInterface->SetActiveness(bBecomeActive); // Set as active
		}
	}
}

// Sets cannon(s) as fireable or not depending on boss force field
void UCannonManagerComponent::ChangeCannonsFireable(EForceFieldChange const ForceFieldChange)
{
	switch (ForceFieldChange)
	{
	case EForceFieldChange::Hit:
		SetCannonFireable(true);
		break;
	case EForceFieldChange::Depleted:
		SetMultipleCannonsFireable(false, ActiveCannons.Num());
		break;
	case EForceFieldChange::Restored:
		SetMultipleCannonsFireable(true, MaxActiveCannons);
		break;
	}
}

// Sets up cannons and cannonball stacks for final wave
void UCannonManagerComponent::OnNewWave(bool const bIsFinalWave)
{
	if (bIsFinalWave)
	{
		SetCannonActivatables(true); // Set cannonballs and stacks as active for final wave
		const UEnemyManagerComponent* EnemyManager = Cast<AGameModeLevel>(GetOwner())->GetEnemyManagementComponent(); // Get reference to enemy manager
		SetCannonsBoss(EnemyManager->GetBossEnemy()); // Set boss enemy for cannons to target
		SetMultipleCannonsFireable(true, MaxActiveCannons); // Set multiple cannons as able to fire at start of final wave
	}
}

// Sets the starting variables for cannon management
void UCannonManagerComponent::SetStartVariables()
{
	TArray<AActor*> TempActors; // Temporary array to store actors to add to arrays
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACannonballStack::StaticClass(), TempActors); // Get all cannonball stacks in level
	
	// Add cannonball stacks to cannonball array
	for (AActor* TempActor : TempActors)
	{
		if (IsValid(TempActor))
		{
			CannonActivatables.Add(Cast<ACannonballStack>(TempActor));
		}
	}
	
	TempActors.Empty(); // Clear array to reuse for cannonballs
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACannonball::StaticClass(), TempActors); // Get all cannonballs in level
	
	// Add cannonballs to cannonball array
	for (AActor* TempActor : TempActors)
	{
		if (IsValid(TempActor))
		{
			CannonActivatables.Add(Cast<ACannonball>(TempActor));
		}
	}
	
	TempActors.Empty(); // Clear array to reuse for cannons
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACannon::StaticClass(), TempActors); // Get all cannons in level
	
	// Add cannons to inactive cannon array
	for (AActor* TempActor : TempActors)
	{
		if (IsValid(TempActor))
		{
			InactiveCannons.Add(Cast<ACannon>(TempActor));
		}
	}
	
	SetCannonActivatables((false)); // Set cannonballs and stacks as inactive at start of level
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

// Sets boss enemy for cannons to target
void UCannonManagerComponent::SetCannonsBoss(ABossEnemy* BossEnemy)
{
	for (ACannon* Cannon : InactiveCannons)
	{
		if (IsValid(Cannon))
		{
			Cannon->SetBoss(BossEnemy);
		}
	}
}