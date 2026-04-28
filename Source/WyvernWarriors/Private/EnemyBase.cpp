#include "EnemyBase.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SplineComponent.h"
#include "EnemyPatrolRoute.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Math/Quat.h"

// Sets default values
AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create and configure capsule component
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	RootComponent = CapsuleComponent;

	// Create and configure skeletal mesh
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(RootComponent);

	// Create and configure floating movement component
	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));
	FloatingPawnMovement->UpdatedComponent = RootComponent;
}

// Moves the enemy along the spline path
void AEnemyBase::MoveAlongSpline(float DeltaTime)
{
	// If no spline component, don't move
	if (!IsValid(SplineComponent))
	{
		return;
	}
	
	DistanceAlongSpline += GetVelocity().Size()	 * DeltaTime; // Update distance along spline based on movement speed
	
	FVector SplineDirection = SplineComponent->GetDirectionAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World); // Get new rotation on spline
	RotateAndMove(SplineDirection, DeltaTime); // Rotate and move enemy along spline
	
	// Reset distance if end of spline reached
	if (DistanceAlongSpline >= SplineComponent->GetSplineLength())
	{
		DistanceAlongSpline = 0.f;
	}
}

/* Checks if grunt is on patrol route and sets relevant status.
 */
void AEnemyBase::CheckOnPatrolRoute()
{	
	bOnRoute = FVector::DistSquared(GetActorLocation(), SplineComponent->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World)) < (2000 * 2000);
}

/* Assigns the patrol route, spline component, distance along spline, and current health of the enemy. Returns early
 * if patrol route is invalid.
 * @param InitialDistance - starting distance along patrol route
 * @param Route - patrol route assigned to enemy
 * @param bSpawnOnRoute - whether the enemy is spawning on the patrol route
 */
void AEnemyBase::InitializeEnemy(float InitialDistance, AEnemyPatrolRoute* Route, bool bSpawnOnRoute)
{
	if (!IsValid(Route))
	{
		return;
	}
	
	PatrolRoute = Route;
	SplineComponent = PatrolRoute->GetSplineComponent();
	DistanceAlongSpline = InitialDistance;
	CurrentHealth = MaxHealth;
}

// Modifies the enemy's health by a specified amount
void AEnemyBase::ModifyCurrentHealth(float const Amount)
{
	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.f, MaxHealth); // Adjust health and clamp between 0 and MaxHealth
	
	// Output if health was reduced to 0 or below
	if (CurrentHealth <= 0)
	{
		DestroySelfEnemy();
	}
}

/* Gets reference to player character
 */
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0); // Get player character
}

/* Finds rotation based on input direction and possible collisions. Keeps rotation from moving enemy beneath 0 then
 * converts to quaternion to use for interpolation. Gets difference between rotation quaternion and current quaternion
 * to slow down speed when difference is large. Adds movement input based on speed.
 * @param Direction - FVector reference of direction that enemy should move
 * @param DeltaTime = float that is the amount of time since last tick
 * @param ActorsToAvoid - Array of actors that the enemy should move away from, empty by default
 */
void AEnemyBase::RotateAndMove(FVector& Direction, const float DeltaTime, const TArray<AActor*>& ActorsToAvoid)
{
	CheckForMovementCollision(Direction, ActorsToAvoid);

	FRotator Rotation = Direction.Rotation();
	
	if (GetActorLocation().Z < .0f && Rotation.Pitch < .0f)
    {
    	Rotation.Pitch *= -1.f;
    }

	FQuat const DesiredQuat = Rotation.Quaternion();
	FQuat const NextRotation = FMath::QInterpTo(GetActorQuat(), DesiredQuat, DeltaTime, 1); 
	SetActorRotation(NextRotation);
	
	float const RotationDifference = GetActorQuat().AngularDistance(DesiredQuat);
	FloatingPawnMovement->MaxSpeed = MaxMovementSpeed / (1 + (RotationDifference * RotationDifference));
	
	AddMovementInput(GetActorForwardVector(), FloatingPawnMovement->MaxSpeed, true);
}

/* Checks if there are actors to avoid, and changes direction to move away from them if so. Averages the direction
 * away from each actor if there are multiple.
 * @param Direction - FVector reference of direction that enemy should move, modified if there are actors to avoid.
 * @param ActorsToAvoid - Array of actors that the enemy should move away from.
 */
void AEnemyBase::CheckForMovementCollision(FVector& Direction, const TArray<AActor*>& ActorsToAvoid) const
{
	if (!ActorsToAvoid.IsEmpty())
	{
		FVector DirectionAwaySum = FVector::Zero();
		for (const AActor* Enemy : ActorsToAvoid)
		{
			FVector const SingleDirection = (GetActorLocation() - Enemy->GetActorLocation()).GetSafeNormal();
			DirectionAwaySum += SingleDirection;
		}
		
		FVector const DirectionAway = DirectionAwaySum / ActorsToAvoid.Num();
		Direction = (Direction + DirectionAway).GetSafeNormal();
	}
}

/* Checks if patrol route spline is valid, then gets direction to spot on patrol route to rotate and move towards.
 * Check if grunt is on patrol route.
 * @param DeltaTime - float that is time since last tick
 */
void AEnemyBase::ReturnToRoute(float const DeltaTime)
{
	if (!IsValid(SplineComponent)) { return; }
	
	FVector const FormerSpotOnRoute = SplineComponent->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World); // Get location for spot on route
	FVector DirectionToSpot = FormerSpotOnRoute - GetActorLocation();
	RotateAndMove(DirectionToSpot, DeltaTime, NearbyEnemies);

	CheckOnPatrolRoute();
}
