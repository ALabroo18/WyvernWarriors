#include "GameManagers/Components/CannonManagerComponent.h"
#include "BossEnemy.h"
#include "CannonballStack.h"
#include "Cannonball.h"
#include "Cannon.h"
#include "GameManagers/GameModeLevel.h"
#include "GameManagers/Components/EnemyManagerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ActivatableInterface.h"
#include "EventBusComponent.h"

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

/* Sets cannon as loadable when boss is hovering and sets unloadable when boss is returning to patrol route.
 * @param NewBossState - The state that the boss is currently in.
 */
void UCannonManagerComponent::ChangeCannonFireable(EBossState const NewBossState)
{
	switch (NewBossState)
	{
	case EBossState::Hovering:
		SetCannonLoadable();
		break;
	case EBossState::ReturningToPatrolRoute:
		SetCannonUnloadable();
		break;
	default:
		break;
	}
}

/* Sets cannonballs and stacks as active and sets some cannons as able to fire at the boss on the final wave. Also sets
 * the boss for the cannons to target. Sets up change cannons fireable to listen to boss force field change delegate.
 * @param bIsFinalWave - bool for if the new wave is the final wave.
 */
void UCannonManagerComponent::OnNewWave(bool const bIsFinalWave)
{
	if (bIsFinalWave)
	{
		UEventBusComponent* EventBus = Cast<AGameModeLevel>(GetOwner())->GetEventBusComponent();
		EventBus->OnBossStateChange.AddDynamic(this, &UCannonManagerComponent::ChangeCannonFireable);
		
		const UEnemyManagerComponent* EnemyManager = Cast<AGameModeLevel>(GetOwner())->GetEnemyManagementComponent();
		ABossEnemy* BossEnemy = EnemyManager->GetBossEnemy();
		SetCannonsBoss(BossEnemy);
		
		SetCannonActivatables(true);
	}
}

// Sets the starting variables for cannon management
void UCannonManagerComponent::SetupCannonManager()
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
			Cannons.Add(Cast<ACannon>(TempActor));
		}
	}
	
	SetCannonActivatables((false)); // Set cannonballs and stacks as inactive at start of level
}

/* Sets the boss enemy reference for all cannons.
 * @param BossEnemy - Reference to the boss enemy.
 */
void UCannonManagerComponent::SetCannonsBoss(ABossEnemy* BossEnemy)
{
	for (ACannon* Cannon : Cannons)
	{
		Cannon->SetBoss(BossEnemy);
	}
}

/* Sets cannon closest to the boss as loadable and sets boss reference. Exits early if there are no cannons.
 */
void UCannonManagerComponent::SetCannonLoadable()
{
	if (Cannons.IsEmpty()) { UE_LOG(LogTemp, Log, TEXT("There are no cannons in the level.")); return; }
	
	ActiveCannon = GetCannonClosestToBoss();
	ActiveCannon->SetLoadable();
}

/* Sets the active cannon as unloadable.
 */
void UCannonManagerComponent::SetCannonUnloadable() const
{
	ActiveCannon->SetUnloadable();
}

/* Loops through cannons and gets their distance to the boss. Sets the cannon and distance when distance is lowest.
 * @return ACannon* - The cannon closest to the boss.
 */
ACannon* UCannonManagerComponent::GetCannonClosestToBoss()
{
	float ClosestDistanceToBoss = BIG_NUMBER;
	ACannon* ClosestToBoss = nullptr;
	for (ACannon* Cannon : Cannons)
	{
		if (float const CannonDistanceToBoss = Cannon->GetDistanceToBossSquared(); CannonDistanceToBoss < ClosestDistanceToBoss)
		{
			ClosestDistanceToBoss = CannonDistanceToBoss;
			ClosestToBoss = Cannon;
		}
	}
	
	return ClosestToBoss;
}