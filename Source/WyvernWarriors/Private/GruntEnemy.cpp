#include "GruntEnemy.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "GameManagers/GameModeLevel.h"
#include "GameManagers/Components/EventBusComponent.h"
#include "EnemyPatrolRoute.h"
#include "GruntEnemyProjectile.h"
#include "GruntEnemyController.h"
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

/* Sets up mesh dynamic material and gets references to player character and camera. Gets event bus component from game
 * mode for broadcasting events on enemy death.
 */
void AGruntEnemy::BeginPlay()
{
	Super::BeginPlay();
	SkeletalMesh->CreateDynamicMaterialInstance(0, SkeletalMesh->GetMaterial(0));
	PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	
	if (const AGameModeLevel* GameModeLevel = Cast<AGameModeLevel>(UGameplayStatics::GetGameMode(GetWorld())); IsValid(GameModeLevel))
	{
		EventBus = GameModeLevel->GetEventBusComponent();
	}
}

/*
 */
void AGruntEnemy::CheckForMovementCollision(FVector& Direction, const TArray<AActor*>& ActorsToAvoid) const
{
	Super::CheckForMovementCollision(Direction, ActorsToAvoid);
	
	// Use large detection sphere for terrain and non-enemy actors (excluding player).
}

/* Toggles the grunt as being active or inactive by setting collisions and movement speed. Also toggles tick and
 * visibility of grunt enemy. Tells controller to unpossess this grunt.
 * @param bIsActive - whether the grunt enemy is being made active or inactive
 */
AGruntEnemyController* AGruntEnemy::ToggleGruntEnemy(bool const bToggleActive)
{
	SetActorTickEnabled(bToggleActive);
	SetActorHiddenInGame(!bToggleActive);
	bIsActive = bToggleActive;
	
	if (bToggleActive)
	{
		CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		
		FloatingPawnMovement->MaxSpeed = MaxMovementSpeed;
		return nullptr;
	}

	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	FloatingPawnMovement->MaxSpeed = 0.f;
	FloatingPawnMovement->StopMovementImmediately();
	
	AGruntEnemyController* GruntEnemyController = Cast<AGruntEnemyController>(GetController());
	if (!IsValid(GruntEnemyController)) { return nullptr; }
	
	GruntEnemyController->UnPossess();
	return GruntEnemyController;

}

/* Spawns projectile at spawn location and plays attack SFX.
 */
void AGruntEnemy::AttackPlayer()
{
	if (!IsValid(PlayerCharacter))
	{
		return;
	}
	
	FActorSpawnParameters ProjectileSpawnParameters;
	ProjectileSpawnParameters.Owner = this;
	GetWorld()->SpawnActor<AGruntEnemyProjectile>(AttackProjectile, AttackProjectileSpawn->GetComponentLocation(), AttackProjectileSpawn->GetComponentRotation(), ProjectileSpawnParameters);
	
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), AttackSFX, GetActorLocation(), AttackSFXVolume);
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

/* Subtracts itself from patrol route count and increase kill combo. Toggle self inactive and broadcasts grunt death
 * event with this grunt.
 */
void AGruntEnemy::DestroySelfEnemy()
{
	if (IsValid(PatrolRoute))
	{
		PatrolRoute->ModifyRouteEnemyCount(false);
	}
	
	IncreaseKillCombo();
	EventBus->OnGruntDeath.Broadcast(this);
	ToggleGruntEnemy(false);
	
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
	DirectionFromPlayer.Y += RetreatDirectionOffset.X;
	DirectionFromPlayer.Z += RetreatDirectionOffset.Y;
	DirectionFromPlayer.Normalize();
	RotateAndMove(DirectionFromPlayer, DeltaTime, NearbyEnemies);
}

/* Set the random offset for the flee direction.
 */
void AGruntEnemy::SetFleeOffset()
{
	RetreatDirectionOffset = FVector2D(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f));
	RetreatDirectionOffset.Normalize();
}

/* Gets controller for this grunt and runs the aggressive behavior subtree on it.
 */
void AGruntEnemy::UseAggressiveTreeOnly() const
{
	AGruntEnemyController* GruntEnemyController = Cast<AGruntEnemyController>(GetController());
	if (!IsValid(GruntEnemyController)) { return; }
	
	GruntEnemyController->RunAggressiveTree();
}
