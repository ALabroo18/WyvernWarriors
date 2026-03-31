#include "GameManagers/Components/EnemyManagerComponent.h"
#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GruntEnemy.h"
#include "BossEnemy.h"
#include "EnemySpawnPoint.h"
#include "EnemyPatrolRoute.h"
#include "GruntEnemyController.h"
#include "Outpost.h"

/* Populates the inactive grunt enemy queue with new grunt enemies up to the spawn capacity.
 */
void UEnemyManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	for (int i = 0; i < GruntSpawnCapacity; i++)
	{
		AGruntEnemy* NewGruntEnemy = GetWorld()->SpawnActor<AGruntEnemy>();
		InactiveGruntEnemies.Enqueue(NewGruntEnemy);
	}
}

// Spawns a single grunt enemy at a random spawn point and patrol route distance, optionally on a specified patrol route
FTransform UEnemyManagerComponent::GetGruntSpawnTransform(AEnemyPatrolRoute* SpecificPatrolRoute, float& DistanceAlongSpline)
{
	FVector SpawnLocation; // Location where the enemy will be spawned
	FRotator SpawnRotation; // Rotation for the spawned enemy

	// Determine spawn location and rotation based on whether a patrol route was specified
	if (IsValid(SpecificPatrolRoute))
	{
		DistanceAlongSpline = FMath::RandRange(0.f, SpecificPatrolRoute->GetComponentByClass<USplineComponent>()->GetSplineLength());
		SpawnLocation = SpecificPatrolRoute->GetComponentByClass<USplineComponent>()->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
		SpawnRotation = SpecificPatrolRoute->GetComponentByClass<USplineComponent>()->GetRotationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
	}
	else
	{
		if (EnemySpawnPoints.IsEmpty())
		{
			return FTransform::Identity;
		}
		
		const AEnemySpawnPoint* SpawnPoint = EnemySpawnPoints[FMath::RandRange(0, EnemySpawnPoints.Num() - 1)]; // Select a random spawn point
		if (!IsValid(SpawnPoint))
		{
			return FTransform::Identity;
		}
		SpawnLocation = SpawnPoint->GetActorLocation(); // Get the location of the spawn point
		SpawnRotation = FRotator(0.f, 0.f, 0.f); // Default rotation for the spawned enemy
	}


	FVector const PlayerLocation = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetActorLocation(); // Get the player's location'
	float const DistanceToPlayer = FVector::Dist(SpawnLocation, PlayerLocation); // Calculate distance from spawn point to player
	
	if (DistanceToPlayer < MinimumPlayerSpawnDistance) 
	{
		return FTransform::Identity;
	}
	
	TArray<AActor*> NearbyEnemies;
	bool const bIsEnemyNearby = UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		SpawnLocation,
		MinimumEnemySpawnDistance, 
		TArray<TEnumAsByte<EObjectTypeQuery>>(),
		AGruntEnemy::StaticClass(), 
		TArray<AActor*>(), 
		NearbyEnemies);

	if (bIsEnemyNearby)
	{
		FVector DirectionAway = FVector::Zero();
		for (const AActor* NearbyEnemy : NearbyEnemies)
		{
			DirectionAway += SpawnLocation - NearbyEnemy->GetActorLocation();
		}
		DirectionAway /= NearbyEnemies.Num();
		SpawnLocation += DirectionAway.GetSafeNormal() * MinimumEnemySpawnDistance;
	}
	return FTransform(SpawnRotation, SpawnLocation, FVector::One()); // Set transform for the spawned enemy
}

/* Loops through all patrol routes and adds open non-boss patrol route to an array. Selects one patrol route from array
 * to return. Returns early if there are no patrol route or open patrol routes.
 * @return AEnemyPatrolRoute = the route for the grunt to spawn on
 */
AEnemyPatrolRoute* UEnemyManagerComponent::GetSpawnPatrolRoute()
{
	if (EnemyPatrolRoutes.IsEmpty())
	{
		return nullptr;
	}
	
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
	
	if (OpenPatrolRoutes.IsEmpty())
	{
		return nullptr;
	}

	return OpenPatrolRoutes[FMath::RandRange(0, OpenPatrolRoutes.Num() - 1)];
}

/* Defers spawning the grunt to initialize grunt, add to grunt array, and increase route grunt count. Spawns grunt at
 * specified transform after. Returns early is spawn transform is invalid, route is invalid, or if grunt spawn capacity
 * has been reached.
 * @param SpawnTransform - transform that the grunt is spawned at
 * @param DistanceAlongSpline - distance along patrol route grunt is spawned with
 * @param Route - patrol route enemy is assigned to on spawn
 * @param bSpawnOnRoute - bool for if grunt spawns on route or not
 */
AGruntEnemy* UEnemyManagerComponent::SpawnGruntEnemy(const FTransform& SpawnTransform, float const DistanceAlongSpline, AEnemyPatrolRoute* Route, bool const bSpawnOnRoute, bool const bIgnoreSpawnCap)
{
	if (!IsValid(Route) || SpawnTransform.Equals(FTransform::Identity))
	{
		return nullptr;
	}
	
	AGruntEnemy* NewGruntEnemy;
	if (bIgnoreSpawnCap)
	{
		NewGruntEnemy = GetWorld()->SpawnActorDeferred<AGruntEnemy>(GruntEnemyToSpawn, SpawnTransform);
	}
	else if (InactiveGruntEnemies.Dequeue(NewGruntEnemy); !IsValid(NewGruntEnemy))
	{
		NewGruntEnemy = GetWorld()->SpawnActorDeferred<AGruntEnemy>(GruntEnemyToSpawn, SpawnTransform);
	}
	
	ActiveGruntEnemies.Add(NewGruntEnemy);
	Route->ModifyEnemiesOnRoute(true);
	
	NewGruntEnemy->InitializeEnemy(DistanceAlongSpline, Route, bSpawnOnRoute); 
	NewGruntEnemy->FinishSpawning(SpawnTransform);
	
	AGruntEnemyController* GruntEnemyController = Cast<AGruntEnemyController>(NewGruntEnemy->Controller);
	GruntEnemyController->StartBehaviorTree(); // Error here
	
	return NewGruntEnemy;
}

/* Spawns a random amount of grunt enemies on outpost patrol route depending on pre-existing amount and add enemies to
 * outpost if not already there. Doesn't spawn if too many grunts or if outpost patrol route is full.
 * @param Outpost - The outpost for which to spawn the grunt enemies
 * @param bIsFinalWave - Whether the current wave is the final wave
 */
void UEnemyManagerComponent::SpawnGruntEnemiesForOutpost(AOutpost* Outpost, bool const bIsFinalWave)
{
	if (ActiveGruntEnemies.Num() >= GruntSpawnCapacity)
	{
		return;
	}
	
	AEnemyPatrolRoute* OutpostPatrolRoute = Outpost->GetOutpostPatrolRoute();
	if (!IsValid(OutpostPatrolRoute) || OutpostPatrolRoute->GetRouteFull())
	{
		return;
	}
	
	int32 const MaxGruntSpawn = OutpostPatrolRoute->GetMaxEnemiesOnRoute() - OutpostPatrolRoute->GetNumEnemiesOnRoute();
	int32 SpawnAmount = FMath::RandRange(MaxGruntSpawn / 2, MaxGruntSpawn);
	
	while (SpawnAmount > 0 && !OutpostPatrolRoute->GetRouteFull())
	{
		float RouteSpawnDistance;
		FTransform const SpawnTransform = GetGruntSpawnTransform(OutpostPatrolRoute, RouteSpawnDistance);
		if (SpawnTransform.Equals(FTransform::Identity))
		{
			continue;
		}
		
		AGruntEnemy* NewGrunt = SpawnGruntEnemy(
			SpawnTransform, 
			RouteSpawnDistance, 
			OutpostPatrolRoute,
			true,
			false);
		if (!Outpost->GetGruntEnemies().Contains(NewGrunt))
		{
			Outpost->AddGruntEnemy(NewGrunt);
		}
		
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
void UEnemyManagerComponent::RemoveActiveGruntEnemy(AGruntEnemy* GruntEnemy)
{
	if (!IsValid(GruntEnemy))
	{
		return;
	}

	ActiveGruntEnemies.Remove(GruntEnemy); // Remove the grunt enemy from the managed array
}

/* Destroys all valid enemies, including grunts and boss if included.
 * @param bIncludeBoss - bool for is boss should be destroyed too.
 */
void UEnemyManagerComponent::DestroyAllEnemies(bool const bIncludeBoss)
{
	if (bIncludeBoss && IsValid(BossEnemy))
	{
		BossEnemy->Destroy();
	}
	
	while (!ActiveGruntEnemies.IsEmpty())
	{
		if (AGruntEnemy* Grunt = ActiveGruntEnemies.Last(); IsValid(Grunt))
		{
			Grunt->DestroySelfEnemy();
		}
	}
}

/* On the final wave, destroy all enemies and spawn the boss.
 */
void UEnemyManagerComponent::OnNewWave(bool const bIsFinalWave)
{
	if (bIsFinalWave)
	{
		DestroyAllEnemies(false);
		SpawnBoss();
	}
}


