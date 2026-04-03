#include "EnemyPatrolRoute.h"
#include "Components/SplineComponent.h"

// Sets default values
AEnemyPatrolRoute::AEnemyPatrolRoute()
{
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent")); // Create spline component
	RootComponent = SplineComponent; // Set spline component as root
}

// Modifies the number of enemies on the patrol route
void AEnemyPatrolRoute::ModifyRouteEnemyCount(bool bIncreaseCount)
{
	// If trying to increase the number of enemies but the route is already full, do nothing
	if (bIncreaseCount && bIsRouteFull)
	{
		return;
	}
	
	NumEnemiesOnRoute += bIncreaseCount ? 1 : -1; // Increase or decrease the number of enemies on the route

	SetRouteFull(); // Set if the patrol route is full after modification
}

// Sets whether the patrol route is full
void AEnemyPatrolRoute::SetRouteFull()
{
	bIsRouteFull = NumEnemiesOnRoute >= MaxEnemiesOnRoute; // Check if the patrol route is full
}

