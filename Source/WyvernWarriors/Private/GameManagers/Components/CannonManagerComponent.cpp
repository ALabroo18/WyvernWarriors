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
	for (AActor* Activatable : CannonballsAndStacks)
	{
		if (IsValid(Activatable))
		{
			IActivatableInterface* ActivatableInterface = Cast<IActivatableInterface>(Activatable); // Cast to activatable interface
			ActivatableInterface->SetActiveness(bBecomeActive); // Set as active
		}
	}
}

/* Sets cannon loadable and cannonballs grabbable when boss enters hover state.
 * @param NewBossState - The state that the boss is currently in.
 */
void UCannonManagerComponent::EnableCannonsAndCannonballs(EBossState const NewBossState)
{
	switch (NewBossState)
	{
	case EBossState::Hovering:
		SetCannonLoadable();
		SetCannonballsGrabbable();
		break;
	default:
		break;
	}
}

/* Sets cannonballs ungrabbable when cannon is loaded.
 */
void UCannonManagerComponent::OnCannonLoaded()
{
	SetCannonballsUngrabbable();
}

/* Disables actives cannon and sets cannonballs ungrabbable when force field is inactive.
 * @param DestroyedVillage - Unused tag of the destroyed village.
 */
void UCannonManagerComponent::OnVillageDestroyed(FName DestroyedVillage)
{
	SetCannonballsUngrabbable();
	SetCannonUnloadable();
}

/* Sets cannonballs and stacks as active. Sets the boss for the cannons to target. Sets up change cannons fireable to
 * listen to boss state change delegate.
 * @param bIsFinalWave - bool for if the new wave is the final wave.
 */
void UCannonManagerComponent::OnNewWave(bool const bIsFinalWave)
{
	if (bIsFinalWave)
	{
		UEventBusComponent* EventBus = Cast<AGameModeLevel>(GetOwner())->GetEventBusComponent();
		EventBus->OnBossStateChange.AddDynamic(this, &UCannonManagerComponent::EnableCannonsAndCannonballs);
		EventBus->OnCannonLoaded.AddDynamic(this, &UCannonManagerComponent::OnCannonLoaded);
		EventBus->OnVillageDestroyed.AddDynamic(this, &UCannonManagerComponent::OnVillageDestroyed);
		
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
			CannonballsAndStacks.Add(Cast<ACannonballStack>(TempActor));
		}
	}
	
	TempActors.Empty(); // Clear array to reuse for cannonballs
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACannonball::StaticClass(), TempActors); // Get all cannonballs in level
	
	// Add cannonballs to cannonball array
	for (AActor* TempActor : TempActors)
	{
		if (IsValid(TempActor))
		{
			ACannonball* TempCannonball = Cast<ACannonball>(TempActor);
			CannonballsAndStacks.Add(TempCannonball);
			Cannonballs.Add(TempCannonball);
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

/* Sets all cannonballs pickup sphere collision as enabled.
 */
void UCannonManagerComponent::SetCannonballsGrabbable()
{
	for (ACannonball* Cannonball : Cannonballs)
	{
		Cannonball->SetPickUpSphereCollision(true);
		Cannonball->ActivatePickUpUI();
		UE_LOG(LogTemp, Log, TEXT("Setting %s cannonball active."), *Cannonball->GetName());
	}
}

/* Sets all cannonballs pickup sphere collision as disabled.
 */
void UCannonManagerComponent::SetCannonballsUngrabbable()
{
	for (ACannonball* Cannonball : Cannonballs)
	{
		Cannonball->SetPickUpSphereCollision(false);
		Cannonball->DeactivatePickUpUI();
	}
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