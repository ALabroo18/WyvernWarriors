#include "Cannon.h"
#include "Cannonball.h"
#include "BossEnemy.h" 
#include "Components/WidgetComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this actor's properties
ACannon::ACannon()
{
	// Set up root box collider
	BoxComponent = CreateDefaultSubobject<UBoxComponent>("Box Component");
	SetRootComponent(BoxComponent);
	
	// Set up the static meshes
	CannonTopMesh = CreateDefaultSubobject<UStaticMeshComponent>("Static Mesh Top");
	CannonTopMesh->SetupAttachment(RootComponent);
	CannonBottomMesh = CreateDefaultSubobject<UStaticMeshComponent>("Static Mesh Bottom");
	CannonBottomMesh->SetupAttachment(RootComponent);
	
	// Set up the widget component
	ReadyToFireWidget = CreateDefaultSubobject<UWidgetComponent>("Ready To Fire Widget");
	ReadyToFireWidget->SetupAttachment(RootComponent);
}

// Rotates the cannon and fires the cannonball at the boss
void ACannon::FireCannonball()
{
	// Return if no boss to shot at
	if (!IsValid(BossEnemy))
	{
		return;
	}
	
	FTransform SpawnTransform(FRotator::ZeroRotator, GetActorLocation(), FVector::One()); // Create spawn transform for cannonball
	
	// Defer spawning cannonball to determine cannon rotation
	ACannonball* NewCannonball = GetWorld()->SpawnActorDeferred<ACannonball>(Cannonball, SpawnTransform);
	if (!IsValid(NewCannonball))
	{
		return;
	}
	
	SpawnTransform.SetRotation(SetFiringRotation(NewCannonball).Quaternion()); // Rotate cannon and set spawn rotation of cannonball
	
	NewCannonball->FinishSpawning(SpawnTransform); // Finish spawning cannonball
	SetFirable(false); // Set cannon as not ready to fire after firing
}

// Set cannon fireable and widget visibility
void ACannon::SetFirable(bool const bCanFire)
{
	bIsReadyToFire = bCanFire; // Set whether the cannon is ready to fire
	ReadyToFireWidget->SetVisibility(bCanFire); // Set widget visibility to match whether the cannon is ready to fire
	
	// Set collision for box collision
	if (bCanFire)
	{
		BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	else
	{
		BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

// Rotate the cannon to fire at where the boss will be
FRotator ACannon::SetFiringRotation(ACannonball* Ball)
{
	FVector const CurrentLocation = GetActorLocation(); // Get actor location
	float const CannonballSpeed = Ball->GetProjectileSpeed(); // Get cannonball speed
	float PreviousDistance = 0.f; // Variable for distance of previous trial
	
	float DistanceToBoss = UKismetMathLibrary::Vector_Distance(CurrentLocation, BossEnemy->GetActorLocation()); // Get distance to current boss location
	float TravelTime = DistanceToBoss / CannonballSpeed; // Get travel time of cannonball to get to current boss location
	FVector TargetLocation = BossEnemy->GetFutureLocation(TravelTime); // Get future boss location after time of travel
	
	// Continuously get travel time to future boss location until same distance is gotten.
	for (int i = 0; i < 75; i++)
	{
		float const PrePreviousDistance = PreviousDistance; // Variable for the distance of the trial before the previous trial
		PreviousDistance = DistanceToBoss; // Update previous trial distance
		DistanceToBoss = UKismetMathLibrary::Vector_Distance(CurrentLocation, TargetLocation); // Set distance to current future boss location
		TravelTime = DistanceToBoss / CannonballSpeed; // Set travel time to get to current future boss location
		TargetLocation = BossEnemy->GetFutureLocation(TravelTime); // Get the next future boss's location given the current travel time
		
		// Break when a future boss's distance matches a previous trial
		if (PreviousDistance == DistanceToBoss || PrePreviousDistance == DistanceToBoss)
		{
			break;
		}
	}
	
	FRotator const RotationTowardsBoss = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetLocation); // Rotation towards's future boss's location
	FRotator CurrentRotation = GetActorRotation(); // Get current cannon rotation
	CurrentRotation.Yaw = RotationTowardsBoss.Yaw; // Change cannon yaw rotation
	SetActorRotation(CurrentRotation); // Rotate entire cannon yaw towards boss
	
	FRotator const CurrentTopRotation = FRotator(0.0, -90.0, -RotationTowardsBoss.Pitch); // Set cannon top rotation to align with boss and bottom
	CannonTopMesh->SetRelativeRotation(CurrentTopRotation); // Rotate cannon top roll towards boss
	return RotationTowardsBoss;
}