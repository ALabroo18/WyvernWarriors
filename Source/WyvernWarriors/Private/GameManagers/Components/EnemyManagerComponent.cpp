#include "GameManagers/Components/EnemyManagerComponent.h"
#include "GameManagers/Components/EventBusComponent.h"
#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GruntEnemy.h"
#include "BossEnemy.h"
#include "EnemySpawnPoint.h"
#include "EnemyPatrolRoute.h"
#include "GruntEnemyController.h"
#include "Outpost.h"
#include "GameManagers/GameModeLevel.h"


// Spawns a single grunt enemy at a random spawn point and patrol route distance, optionally on a specified patrol route
FTransform UEnemyManagerComponent::GetGruntSpawnTransform(AEnemyPatrolRoute* SpecificPatrolRoute, float& DistanceAlongSpline)
{
	FVector SpawnLocation = FVector::Zero(); // Location where the enemy will be spawned
	FRotator SpawnRotation; // Rotation for the spawned enemy
	bool bIsPlayerTooClose = true;
	int SpawnAttempts = 0;
	
	while (bIsPlayerTooClose)
	{
		// Determine spawn location and rotation based on whether a patrol route was specified
		if (IsValid(SpecificPatrolRoute))
		{
			DistanceAlongSpline = FMath::RandRange(0.f, SpecificPatrolRoute->GetComponentByClass<USplineComponent>()->GetSplineLength());
			SpawnLocation = SpecificPatrolRoute->GetComponentByClass<USplineComponent>()->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
			SpawnRotation = SpecificPatrolRoute->GetComponentByClass<USplineComponent>()->GetRotationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
		}
		else
		{
			if (EnemySpawnPoints.IsEmpty()) { return FTransform::Identity; }
		
			const AEnemySpawnPoint* SpawnPoint = EnemySpawnPoints[FMath::RandRange(0, EnemySpawnPoints.Num() - 1)]; // Select a random spawn point
			if (!IsValid(SpawnPoint)) { return FTransform::Identity; }
			
			SpawnLocation = SpawnPoint->GetActorLocation(); // Get the location of the spawn point
			SpawnRotation = FRotator(0.f, 0.f, 0.f); // Default rotation for the spawned enemy
		}
		
		FVector const PlayerLocation = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetActorLocation(); // Get the player's location'
		if (float const DistanceToPlayer = FVector::Dist(SpawnLocation, PlayerLocation); DistanceToPlayer < MinimumPlayerSpawnDistance) 
		{
			SpawnAttempts++;
		}
		else
		{
			bIsPlayerTooClose = false;
		}
		
		if (SpawnAttempts >= 10) { return FTransform::Identity; }
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
	if (EnemyPatrolRoutes.IsEmpty()) { return nullptr; }
	
	TArray<AEnemyPatrolRoute*> OpenPatrolRoutes;
	for (AEnemyPatrolRoute* Route : EnemyPatrolRoutes)
	{
		if (IsValid(Route))
		{
			if (Route->GetRouteFull()) { continue; }

			OpenPatrolRoutes.Add(Route);
		}
	}
	
	if (OpenPatrolRoutes.IsEmpty()) { return nullptr; }

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
AGruntEnemy* UEnemyManagerComponent::SpawnGruntEnemy(const FTransform& SpawnTransform, float const DistanceAlongSpline, AEnemyPatrolRoute* Route, bool const bSpawnOnRoute)
{
	if (ActiveGruntEnemies.Num() >= GruntSpawnCapacity) { return nullptr; }
	if (!IsValid(Route)) { return nullptr; }
	if (SpawnTransform.Equals(FTransform::Identity)) { return nullptr; }

	AGruntEnemy* NewGruntEnemy;
	if (!InactiveGruntEnemies.Dequeue(NewGruntEnemy)) { return nullptr; }

	
	ActiveGruntEnemies.Add(NewGruntEnemy);
	Route->ModifyRouteEnemyCount(true);
	NewGruntEnemy->SetActorTransform(SpawnTransform);
	NewGruntEnemy->InitializeEnemy(DistanceAlongSpline, Route, bSpawnOnRoute);
	NewGruntEnemy->ToggleGruntEnemy(true);

	AGruntEnemyController* NewGruntEnemyController;
	InactiveGruntEnemyControllers.Dequeue(NewGruntEnemyController);
	if (!IsValid(NewGruntEnemyController)) { return nullptr; }
	
	NewGruntEnemyController->Possess(NewGruntEnemy);
	return NewGruntEnemy;
}

/* Populates the inactive grunt enemy queue with new grunt enemies up to the spawn capacity. Gets boss patrol route,
 * grunt patrol routes, and grunt spawn points.
 */
void UEnemyManagerComponent::SetupEnemyManager()
{
	for (int i = 0; i < GruntSpawnCapacity; i++)
	{
		AGruntEnemy* NewGruntEnemy = GetWorld()->SpawnActor<AGruntEnemy>(GruntEnemyToSpawn);
		InactiveGruntEnemies.Enqueue(NewGruntEnemy);
		InactiveGruntEnemyControllers.Enqueue(NewGruntEnemy->ToggleGruntEnemy(false));
	}
	
	TArray<AActor*> TempActors;
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), AEnemyPatrolRoute::StaticClass(), "Grunt", TempActors); // Get all cannonball stacks in level
	
	for (AActor* TempActor : TempActors)
	{
		if (IsValid(TempActor))
		{
			EnemyPatrolRoutes.Add(Cast<AEnemyPatrolRoute>(TempActor));
		}
	}
	
	TempActors.Empty();
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), AEnemyPatrolRoute::StaticClass(), "Boss", TempActors); // Get all cannonball stacks in level
	
	for (AActor* TempActor : TempActors)
	{
		if (IsValid(TempActor))
		{
			BossPatrolRoute = Cast<AEnemyPatrolRoute>(TempActor);
		}
	}
	
	TempActors.Empty();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemySpawnPoint::StaticClass(), TempActors); // Get all cannonball stacks in level
	
	for (AActor* TempActor : TempActors)
	{
		if (IsValid(TempActor))
		{
			EnemySpawnPoints.Add(Cast<AEnemySpawnPoint>(TempActor));
		}
	}
}

/* Spawns a random amount of grunt enemies on outpost patrol route depending on pre-existing amount and add enemies to
 * outpost if not already there. Doesn't spawn if too many grunts or if outpost patrol route is full.
 * @param Outpost - The outpost for which to spawn the grunt enemies
 * @return bool = If grunts were able to spawn at the outpost.
 */
bool UEnemyManagerComponent::SpawnGruntEnemiesForOutpost(AOutpost* Outpost)
{
	
	AEnemyPatrolRoute* OutpostPatrolRoute = Outpost->GetOutpostPatrolRoute();
	if (!IsValid(OutpostPatrolRoute)) { return false; }
	
	int32 const MaxGruntSpawn = OutpostPatrolRoute->GetMaxEnemiesOnRoute() - OutpostPatrolRoute->GetNumEnemiesOnRoute();
	int32 SpawnAmount = FMath::RandRange(MaxGruntSpawn / 2, MaxGruntSpawn);
	int32 SpawnAttempts = 0;
	bool bSuccessfulSpawn = false;
	
	while (SpawnAmount > 0 && !OutpostPatrolRoute->GetRouteFull() && ActiveGruntEnemies.Num() <= GruntSpawnCapacity)
	{
		float RouteSpawnDistance;
		FTransform SpawnTransform;
		if (SpawnAttempts >= 10)
		{
			return bSuccessfulSpawn;
		}
		else
		{
			SpawnTransform = GetGruntSpawnTransform(OutpostPatrolRoute, RouteSpawnDistance);
		}
		
		if (SpawnTransform.Equals(FTransform::Identity))
		{
			SpawnAttempts++;
			continue;
		}
		
		AGruntEnemy* NewGrunt = SpawnGruntEnemy(
			SpawnTransform, 
			RouteSpawnDistance, 
			OutpostPatrolRoute,
			true);
		
		if (!Outpost->GetGruntEnemies().Contains(NewGrunt))
		{
			Outpost->AddGruntEnemy(NewGrunt);
		}

		SpawnAmount--;
		bSuccessfulSpawn = true;
	}
	
	return bSuccessfulSpawn;
 }

// Spawns boss enemy at the boss route
void UEnemyManagerComponent::SpawnBoss()
{
	// Do not spawn if no spawn points available
	if (!IsValid(BossPatrolRoute)) { return; }

	// Ensure the boss route has a valid spline component
	const USplineComponent* BossSpline = BossPatrolRoute->GetComponentByClass<USplineComponent>();
	if (!IsValid(BossSpline)) { return; }

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

/* Removes grunt enemy from active array, then adds grunt enemy and its controller to respective inactive queues.
 * @param GruntEnemy - grunt enemy that is turning inactive
 */
void UEnemyManagerComponent::RemoveActiveGruntEnemy(AGruntEnemy* DeadGrunt)
{
	if (!IsValid(DeadGrunt))
	{
		return;
	}

	ActiveGruntEnemies.Remove(DeadGrunt);
	InactiveGruntEnemies.Enqueue(DeadGrunt);
	InactiveGruntEnemyControllers.Enqueue(Cast<AGruntEnemyController>(DeadGrunt->GetController()));
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

/* Binds the grunt death delegate to remove active grunt function.
 */
void UEnemyManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (const AGameModeLevel* GameModeLevel = Cast<AGameModeLevel>(UGameplayStatics::GetGameMode(GetWorld())); IsValid(GameModeLevel))
	{
		EventBus = GameModeLevel->GetEventBusComponent();
		EventBus->OnGruntDeath.AddDynamic(this, &UEnemyManagerComponent::RemoveActiveGruntEnemy);
	}
}


