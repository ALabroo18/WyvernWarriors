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
#include "Components/CapsuleComponent.h"
#include "GameFramework/FloatingPawnMovement.h"


// Creates components on enemy and sets attachment
AGruntEnemy::AGruntEnemy()
{
	// Create and configure attack projectile spawn point
	AttackProjectileSpawn = CreateDefaultSubobject<UArrowComponent>(TEXT("AttackProjectileSpawn"));
	AttackProjectileSpawn->SetupAttachment(SkeletalMesh, TEXT("Tongue1Socket"));
	
	// Create and configure detection sphere
	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(RootComponent);
	
	// Create and configure health bar widget
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(SkeletalMesh);
}

/* Sets health bar fill percent and on route variable of grunt enemy.
 * @param InitialDistance - starting distance along patrol route
 * @param Route - patrol route assigned to enemy
 * @param bSpawnOnRoute - whether the enemy is spawning on the patrol route
 */
void AGruntEnemy::InitializeEnemy(float const InitialDistance, AEnemyPatrolRoute* Route, bool const bSpawnOnRoute)
{
	Super::InitializeEnemy(InitialDistance, Route, bSpawnOnRoute);
	
	SetHealthBarPercent();
	bOnRoute = bSpawnOnRoute;
}

/* Sets up mesh dynamic material and gets references to player character and camera.
 */
void AGruntEnemy::BeginPlay()
{
	Super::BeginPlay();
	SkeletalMesh->CreateDynamicMaterialInstance(0, SkeletalMesh->GetMaterial(0));
	PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0); 
}

/* Toggles the grunt as being active or inactive by setting collisions and movement speed. Also toggles tick and
 * visibility of grunt enemy.
 * @param bIsActive - whether the grunt enemy is being made active or inactive
 */
void AGruntEnemy::ToggleGruntEnemy(bool const bToggleActive)
{
	if (bToggleActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("Enabling %s grunt"), *this->GetName());
		CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		
		FloatingPawnMovement->MaxSpeed = MaxMovementSpeed;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Disabling %s grunt"), *this->GetName());
		CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DetectionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		FloatingPawnMovement->MaxSpeed = 0.f;
		FloatingPawnMovement->StopMovementImmediately();
		
		GetController()->UnPossess();
	}
	
	SetActorTickEnabled(bToggleActive);
	SetActorHiddenInGame(!bToggleActive);
	bIsActive = bToggleActive;
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

/* Unposses controller from self then tells enemy manager to remove grunt enemy from active array. Subtracts itself
 * from patrol route count then toggles self to become inactive.
 */
void AGruntEnemy::DestroySelfEnemy()
{
	const AGameModeLevel* GameMode = Cast<AGameModeLevel>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!IsValid(GameMode))
	{
		return;
	}
	
	if (UEnemyManagerComponent* EnemyManagementComponent = GameMode->GetEnemyManagementComponent(); IsValid(EnemyManagementComponent))
	{
		EnemyManagementComponent->RemoveActiveGruntEnemy(this);
	}
	
	if (IsValid(PatrolRoute))
	{
		PatrolRoute->ModifyRouteEnemyCount(false);
	}
	
	ToggleGruntEnemy(false);
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