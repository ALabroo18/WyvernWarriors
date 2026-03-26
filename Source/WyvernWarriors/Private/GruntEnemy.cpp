#include "GruntEnemy.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Character.h"
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

/* Checks if patrol route spline is valid, then gets direction to spot on patrol route to rotate and move towards.
 * Check if grunt is on patrol route.
 * @param DeltaTime - float that is time since last tick
 */
void AGruntEnemy::ReturnToRoute(float const DeltaTime)
{
	if (!IsValid(SplineComponent))
	{
		return;
	}

	FVector const FormerSpotOnRoute = SplineComponent->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World); // Get location for spot on route
	FVector DirectionToSpot = FormerSpotOnRoute - GetActorLocation();
	RotateAndMove(DirectionToSpot, DeltaTime, NearbyEnemies);

	CheckOnPatrolRoute();
}

/* Checks if grunt is on patrol route and sets relevant status
 */
void AGruntEnemy::CheckOnPatrolRoute()
{	
	if (FVector::DistSquared(GetActorLocation(), SplineComponent->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World)) < 2000 * 2000)
	{
		bOnRoute = true;
	}
	else
	{
		bOnRoute = false;
	}
}

/* Gets direction towards player than rotates and moves said direction.
 * @param DeltaTime - float that is time since last tick
 */
void AGruntEnemy::ChasePlayerCharacter(float const DeltaTime)
{
	FVector DirectionToPlayer = (PlayerCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	RotateAndMove(DirectionToPlayer, DeltaTime, NearbyEnemies);
}

/* Gets direction away from player than rotates and moves said direction.
 * @param DeltaTime - float that is time since last tick
 */
void AGruntEnemy::FleePlayerCharacter(float const DeltaTime)
{
	FVector DirectionFromPlayer = (GetActorLocation() - PlayerCharacter->GetActorLocation()).GetSafeNormal();
	RotateAndMove(DirectionFromPlayer, DeltaTime, NearbyEnemies);
}