#include "GruntEnemy.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameManagers/GameModeLevel.h"
#include "GameManagers/Components/EnemyManagerComponent.h"
#include "EnemyPatrolRoute.h"
#include "GruntEnemyProjectile.h"


// Creates components on enemy and sets attachment
AGruntEnemy::AGruntEnemy()
{
	// Create and configure attack projectile spawn point
	AttackProjectileSpawn = CreateDefaultSubobject<UArrowComponent>(TEXT("AttackProjectileSpawn"));
	AttackProjectileSpawn->SetupAttachment(SkeletalMesh, TEXT("Tounge1Socket"));
	
	// Create and configure detection sphere
	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(RootComponent);
	
	// Create and configure health bar widget
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(SkeletalMesh);
}

// Initializes enemy variables
void AGruntEnemy::InitializeEnemy(float InitialDistance, AEnemyPatrolRoute* Route, bool bSpawnOnRoute)
{
	Super::InitializeEnemy(InitialDistance, Route, bSpawnOnRoute); // Call base class initialization

	// Start moving along the route if spawned on it
	if (bSpawnOnRoute)
	{
		bOnRoute = true;
	}
}

// Sets enemy variables when spawning 
void AGruntEnemy::SetVariables()
{
	Super::SetVariables();
	SkeletalMesh->CreateDynamicMaterialInstance(0, SkeletalMesh->GetMaterial(0)); // Create dynamic material instance for visual effects
	PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0); // Get player camera manager
}

// Executes the attack on the player by spawning a projectile
void AGruntEnemy::AttackPlayer()
{
	// Exit if player character is not valid
	if (!IsValid(PlayerCharacter))
	{
		return;
	}

	// Spawn projectile with grunt as owner
	FActorSpawnParameters ProjectileSpawnParameters;
	ProjectileSpawnParameters.Owner = this;
	GetWorld()->SpawnActor<AGruntEnemyProjectile>(AttackProjectile, AttackProjectileSpawn->GetComponentLocation(), AttackProjectileSpawn->GetComponentRotation(), ProjectileSpawnParameters);
}

// Orients the health bar to face the player camera when within a certain distance
void AGruntEnemy::TurnHealthBarTowardsPlayer() const
{
	// Ensure player character is valid
	if (!IsValid(PlayerCameraManager))
	{
		return;
	}
	
	FVector const PlayerCameraLocation = PlayerCameraManager->GetCameraLocation(); // Get player location
	float const DistanceToPlayerCamera = FVector::Dist(PlayerCameraLocation, GetActorLocation()); // Calculate distance to player

	// Rotate health bar to face player if within specified distance
	if (DistanceToPlayerCamera <= HealthBarRotationDistance)
	{
		FVector const HealthBarLocation = HealthBarWidget->GetComponentLocation(); // Get health bar location
		FRotator const LookAtRotation = (PlayerCameraLocation - HealthBarLocation).Rotation(); // Calculate rotation to face player
		HealthBarWidget->SetWorldRotation(LookAtRotation); // Set health bar rotation to face player
	}
}

// Highlights or unhighlights the grunt enemy for visual feedback
void AGruntEnemy::HighlightGruntEnemy(bool bHighlight)
{
	// Ensure skeletal mesh is valid
	if (!IsValid(SkeletalMesh))
	{
		return;
	}

	if (bHighlight)
	{
		SkeletalMesh->SetOverlayMaterial(HighlightMaterial); // Set overlay material for highlighting
	}
	else
	{
		SkeletalMesh->SetOverlayMaterial(nullptr); // Remove overlay material to unhighlight
	}
}

// Destroys the grunt enemy actor after removing references
void AGruntEnemy::DestroySelfEnemy()
{
	// Get reference to the game mode
	AGameModeLevel* GameMode = Cast<AGameModeLevel>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!IsValid(GameMode))
	{
		return;
	}

	// Get reference to the enemy management component
	UEnemyManagerComponent* EnemyManagementComponent = GameMode->GetEnemyManagementComponent();
	if (IsValid(EnemyManagementComponent))
	{
		EnemyManagementComponent->RemoveGruntEnemy(this); // Notify enemy management component of destruction
	}

	// Remove enemy from patrol route if valid
	if (IsValid(PatrolRoute))
	{
		PatrolRoute->ModifyEnemiesOnRoute(false); // Remove enemy from patrol route
	}
	
	// do something with egg
	if (bIsEggThief)
	{
		// do something with egg
	}
	
	Destroy(); // Destroy grunt enemy
}

// Rotates and moves grunt back towards patrol route spot
void AGruntEnemy::ReturnToRoute(float DeltaTime)
{
	// Check spline component validity
	if (!IsValid(SplineComponent))
	{
		return;
	}

	FVector FormerSpotOnRoute = SplineComponent->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World); // Get location for spot on route
	FRotator RotationTowardsSpot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), FormerSpotOnRoute); // Get rotation to look at spot
	RotateAndMove(RotationTowardsSpot, DeltaTime); // Rotate and move enemy

	SetOnPatrolRoute(); // Check and set on patrol route bool if necessary
}

// Sets grunt on patrol route if close enough
void AGruntEnemy::SetOnPatrolRoute()
{	
	// Return true if close enough to patrol route spot
	if (FVector::DistSquared(GetActorLocation(), SplineComponent->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World)) < 2000 * 2000)
	{
		bOnRoute = true;
	}
	else
	{
		bOnRoute = false;
	}
}

// Move towards player character while avoiding nearby enemies
void AGruntEnemy::ChasePlayerCharacter(float const DeltaTime)
{
	FVector DirectionAway = FVector::ZeroVector; // Direction away from all nearby enemies
	
	// Get direction away from nearby enemies if any
	if(!NearbyEnemies.IsEmpty())
	{
		FVector DirectionAwaySum; // Sum of away directions to all nearby enemies

		// Add all away directions together
		for (const AGruntEnemy* Enemy : NearbyEnemies)
		{
			FVector const SingleDirection = (GetActorLocation() - Enemy->GetActorLocation()).GetSafeNormal();
			DirectionAwaySum += SingleDirection;
		}
		
		DirectionAway = DirectionAwaySum / NearbyEnemies.Num(); // Get average of away directions
	}
	
	FVector const DirectionToPlayer = (PlayerCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal(); // Get direction to player
	FVector  const DirectionToMove = (DirectionToPlayer + DirectionAway).GetSafeNormal(); // Combine direction to player and direction away from enemies
	FRotator DirectionToRotate = DirectionToMove.Rotation(); // Get rotation from movement direction
	RotateAndMove(DirectionToRotate, DeltaTime); // Rotate and move using rotation
}

// Flee from the character until far enough
void AGruntEnemy::FleePlayerCharacter(float const DeltaTime)
{
	FVector DirectionAway = FVector::ZeroVector; // Direction away from all nearby enemies
	
	// Get direction away from nearby enemies if any
	if(!NearbyEnemies.IsEmpty())
	{
		FVector DirectionAwaySum; // Sum of away directions to all nearby enemies

		// Add all away directions together
		for (const AGruntEnemy* Enemy : NearbyEnemies)
		{
			FVector const SingleDirection = (GetActorLocation() - Enemy->GetActorLocation()).GetSafeNormal();
			DirectionAwaySum += SingleDirection;
		}
		
		DirectionAway = DirectionAwaySum / NearbyEnemies.Num(); // Get average of away directions
	}
	
	FVector const DirectionFromPlayer = (GetActorLocation() - PlayerCharacter->GetActorLocation()).GetSafeNormal(); // Get direction to player
	FVector  const DirectionToMove = (DirectionFromPlayer + DirectionAway).GetSafeNormal(); // Combine direction to player and direction away from enemies
	FRotator DirectionToRotate = DirectionToMove.Rotation(); // Get rotation from movement direction
	RotateAndMove(DirectionToRotate, DeltaTime); // Rotate and move using rotation
}
