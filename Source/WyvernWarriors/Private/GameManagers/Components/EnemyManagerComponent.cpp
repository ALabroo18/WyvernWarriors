#include "GameManagers/Components/EnemyManagerComponent.h"
#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GruntEnemy.h"
#include "BossEnemy.h"
#include "EnemySpawnPoint.h"
#include "EnemyPatrolRoute.h"
#include "Outpost.h"
#include "GameManagers/GameModeLevel.h"
#include "GameManagers/Components/CannonManagerComponent.h"

// Spawns a single grunt enemy at a random spawn point and patrol route distance, optionally on a specified patrol route
AGruntEnemy* UEnemyManagerComponent::SpawnGruntEnemy(AEnemyPatrolRoute* SpecificPatrolRoute)
{
	// Do not spawn if at max capacity
	if (GruntEnemies.Num() >= GruntSpawnCapacity)
	{
		return nullptr;
	}

	bool const bPatrolRouteSpecified = IsValid(SpecificPatrolRoute); // Check if a patrol route was specified
	
	// If no patrol route specified, select a random valid patrol route
	if (!bPatrolRouteSpecified)
	{
		// Do not spawn if no spawn points available
		if (EnemySpawnPoints.IsEmpty())
		{
			return nullptr;
		}
		
		// Filter for valid patrol routes that are not full
		TArray<AEnemyPatrolRoute*> OpenPatrolRoutes;
		for (AEnemyPatrolRoute* Route : EnemyPatrolRoutes)
		{
			if (IsValid(Route))
			{
				// Skip full patrol routes and boss patrol route
				if (Route->GetRouteFull() || Route->ActorHasTag("Boss"))
				{
					continue;
				}

				OpenPatrolRoutes.Add(Route);
			}
		}

		// If no valid patrol routes are available, exit
		if (OpenPatrolRoutes.IsEmpty())
		{
			return nullptr;
		}

		SpecificPatrolRoute = OpenPatrolRoutes[FMath::RandRange(0, OpenPatrolRoutes.Num() - 1)]; // Select a random valid patrol route
	}
	else
	{
		// Ensure the specified patrol route is not full
		if (SpecificPatrolRoute->GetRouteFull())
		{
			return nullptr;
		}
	}

	// Ensure the patrol route has a valid spline component
	if (!IsValid(SpecificPatrolRoute->GetComponentByClass<USplineComponent>()))
	{
		return nullptr;
	}
	
	FVector SpawnLocation; // Location where the enemy will be spawned
	FRotator SpawnRotation; // Rotation for the spawned enemy
	float const DistanceAlongSpline = FMath::RandRange(0.f, SpecificPatrolRoute->GetComponentByClass<USplineComponent>()->GetSplineLength()); // Determine a random distance along the spline for grunt initialization

	// Determine spawn location and rotation based on whether a patrol route was specified
	if (bPatrolRouteSpecified)
	{
		SpawnLocation = SpecificPatrolRoute->GetComponentByClass<USplineComponent>()->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World); // Get the location at the random distance along the spline
		SpawnRotation = SpecificPatrolRoute->GetComponentByClass<USplineComponent>()->GetRotationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World); // Get the rotation at the spawn location
	}
	else
	{
		const AEnemySpawnPoint* SpawnPoint = EnemySpawnPoints[FMath::RandRange(0, EnemySpawnPoints.Num() - 1)]; // Select a random spawn point
		if (!IsValid(SpawnPoint))
		{
			return nullptr;
		}

		SpawnLocation = SpawnPoint->GetActorLocation(); // Get the location of the spawn point
		SpawnRotation = FRotator(0.f, 0.f, 0.f); // Default rotation for the spawned enemy
	}

	FVector const PlayerLocation = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetActorLocation(); // Get the player's location
	float const DistanceToPlayer = FVector::Dist(SpawnLocation, PlayerLocation); // Calculate distance from spawn point to player
	
	// Ensure spawn point is not close to player
	if (DistanceToPlayer < MinimumPlayerSpawnDistance) 
	{
		return nullptr;
	}

	FTransform const SpawnTransform(SpawnRotation, SpawnLocation, FVector::One()); // Set transform for the spawned enemy
	AGruntEnemy* NewGruntEnemy = GetWorld()->SpawnActorDeferred<AGruntEnemy>(GruntEnemyToSpawn, SpawnTransform); // Set up spawn for grunt enemy
	
	// Check for new grunt validity
	if (!IsValid(NewGruntEnemy))
	{
		return nullptr;
	}

	NewGruntEnemy->InitializeEnemy(DistanceAlongSpline, SpecificPatrolRoute, bPatrolRouteSpecified); // Initialize the enemy on the route
	GruntEnemies.Add(NewGruntEnemy); // Add the new enemy to the managed array
	SpecificPatrolRoute->ModifyEnemiesOnRoute(true); // Increment the enemy count on the patrol route
	NewGruntEnemy->FinishSpawning(SpawnTransform); // Spawn the grunt enemy
	return NewGruntEnemy;
}

// Spawns multiple grunt enemies for an outpost, with special handling for the final wave
void UEnemyManagerComponent::SpawnGruntEnemiesForOutpost(AOutpost* outpost, bool bIsFinalWave)
{
	// Do not spawn if at max capacity
	if (GruntEnemies.Num() >= GruntSpawnCapacity)
	{
		return;
	}

	// Get the patrol route for the outpost
	AEnemyPatrolRoute* OutpostPatrolRoute = outpost->GetOutpostPatrolRoute();
	if (!IsValid(OutpostPatrolRoute))
	{
		return;
	}
	
	// Determine spawn amount based on whether it's the final wave
	int32 SpawnAmount = bIsFinalWave
		? FMath::Max(0, 2 - OutpostPatrolRoute->GetNumEnemiesOnRoute())
		: FMath::RandRange(2, 4);

	// Spawn enemies while respecting spawn capacity and route limits
	while (SpawnAmount > 0 && !OutpostPatrolRoute->GetRouteFull())
	{
		SpawnGruntEnemy(OutpostPatrolRoute);
		SpawnAmount--;
	}
 }

// Spawns boss enemy at the boss route
void UEnemyManagerComponent::SpawnBoss()
{
	// Do not spawn if no spawn points available
	if (!IsValid(BossPatrolRoute))
	{
		return;
	}

	// Ensure the boss route has a valid spline component
	const USplineComponent* BossSpline = BossPatrolRoute->GetComponentByClass<USplineComponent>();
	if (!IsValid(BossSpline))
	{
		return;
	}

	float const DistanceAlongSpline = FMath::RandRange(0.f, BossSpline->GetSplineLength()); // Determine a random distance along the spline for boss spawning
	FVector const SpawnLocation = BossSpline->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World); // Determine a random distance along the spline for grunt initalization
	FRotator const SpawnRotation = BossSpline->GetRotationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World); // Get the rotation at the spawn location
	FTransform const SpawnTransform(SpawnRotation, SpawnLocation, FVector::One()); // Create Spawn transform for the boss enemy

	BossEnemy = GetWorld()->SpawnActorDeferred<ABossEnemy>(BossEnemyToSpawn, SpawnTransform); // Defer spawning the boss enemy at the spawn transform

	if (!IsValid(BossEnemy))
	{
		return;
	}

	BossEnemy->InitializeEnemy(DistanceAlongSpline, BossPatrolRoute, true); // Initialize the boss on the route
	BossEnemy->FinishSpawning(SpawnTransform); // Spawn boss after setting variables
}

// Removes a grunt enemy from management
void UEnemyManagerComponent::RemoveGruntEnemy(AGruntEnemy* GruntEnemy)
{
	// Ensure the grunt enemy is valid
	if (!IsValid(GruntEnemy))
	{
		return;
	}

	GruntEnemies.Remove(GruntEnemy); // Remove the grunt enemy from the managed array
}

// Destroys all enemies (grunts, boss) that are alive
void UEnemyManagerComponent::DestroyAllEnemies()
{
	// Check for boss enemy and destroy if possible
	if (!IsValid(BossEnemy))
	{
		BossEnemy->Destroy();
	}
	
	// Check all grunt enemies in array and destroy if possible
	for (AGruntEnemy* Grunt : GruntEnemies)
	{
		if (!IsValid(Grunt))
		{
			Grunt->Destroy();
		}
	}
}

// Spawns boss on final wave and set delegate
void UEnemyManagerComponent::OnNewWave(bool const bIsFinalWave)
{
	if (bIsFinalWave)
	{
		SpawnBoss(); // Spawn boss for final wave
		GameModeLevel = Cast<AGameModeLevel>(GetOwner()); // Get reference to game mode level
		UCannonManagerComponent* CannonManager = GameModeLevel->GetCannonManager(); // Get reference to cannon manager
		BossEnemy->OnForceFieldChange.AddDynamic(CannonManager, &UCannonManagerComponent::ChangeCannonsFireable); // Set delegate for boss force field to change cannon ability to fire
	}
}