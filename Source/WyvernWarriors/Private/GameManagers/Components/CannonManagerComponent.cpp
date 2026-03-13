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
		SetCannonLoadable(true);
		break;
	case EForceFieldChange::Depleted:
		SetMultipleCannonsLoadable(false, ActiveCannons.Num());
		break;
	case EForceFieldChange::Restored:
		SetMultipleCannonsLoadable(true, MaxActiveCannons);
		break;
	}
}

/* Sets cannonballs and stacks as active and sets some cannons as able to fire at the boss at the start of the final
 * wave. Also sets the boss for the cannons to target. 
 */
void UCannonManagerComponent::OnNewWave(bool const bIsFinalWave)
{
	if (bIsFinalWave)
	{
		SetCannonActivatables(true);
		const UEnemyManagerComponent* EnemyManager = Cast<AGameModeLevel>(GetOwner())->GetEnemyManagementComponent();
		SetCannonsBoss(EnemyManager->GetBossEnemy());
		SetMultipleCannonsLoadable(true, MaxActiveCannons);
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


/* Sets a random cannon as able to be loaded or not. Moves cannon between active and inactive arrays and sets cannon
 * as loadable based on input.
 * @param bCanLoad - Whether the cannon should be able to be loaded or not
 */
void UCannonManagerComponent::SetCannonLoadable(bool const bCanload)
{
	if (InactiveCannons.IsEmpty())
	{
		return;
	}
	
	ACannon* CannonToFire;
	
	if (bCanload)
	{
	    CannonToFire = InactiveCannons[FMath::RandRange(0, InactiveCannons.Num() - 1)];
		InactiveCannons.Remove(CannonToFire);
		ActiveCannons.Add(CannonToFire);
	}
	else
	{
	    CannonToFire = ActiveCannons[FMath::RandRange(0, ActiveCannons.Num() - 1)];
		ActiveCannons.Remove(CannonToFire);
		InactiveCannons.Add(CannonToFire);
	}
	
	CannonToFire->SetLoadable(bCanload);
}

/* Sets multiple cannons as able to be loaded.
 * @param bCanLoad - Whether the cannons should be able to be loaded or not
 * @param NumberOfCannons - The number of cannons to set as able to be loaded
*/
void UCannonManagerComponent::SetMultipleCannonsLoadable(bool const bCanLoad, int32 const NumberOfCannons)
{
	for (int i = 0; i < NumberOfCannons; i++)
	{
		SetCannonLoadable(bCanLoad);
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