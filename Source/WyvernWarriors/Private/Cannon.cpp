#include "Cannon.h"
#include "Cannonball.h"
#include "BossEnemy.h" 
#include "Components/WidgetComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "GameManagers/GameModeLevel.h"
#include "GameManagers/Components/EventBusComponent.h"
#include "Kismet/KismetMathLibrary.h"

/* Sets up physics and detection collision components, static meshes for cannon top and bottoms, and UI widget
 * component for cannon fireabiltiy. Binds delegates on collisions components.
 */
ACannon::ACannon()
{
	CannonCollision = CreateDefaultSubobject<UBoxComponent>("Box Component");
	SetRootComponent(CannonCollision);
	
	CannonballDetection = CreateDefaultSubobject<USphereComponent>("Cannonball Detection Sphere");
	CannonballDetection->SetupAttachment(RootComponent);
	CannonballDetection->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CannonballDetection->OnComponentBeginOverlap.AddDynamic(this, &ACannon::OnCannonOverlapBegin);
	CannonballDetection->OnComponentEndOverlap.AddDynamic(this, &ACannon::OnCannonOverlapEnd);
	
	CannonTopMesh = CreateDefaultSubobject<UStaticMeshComponent>("Static Mesh Top");
	CannonTopMesh->SetupAttachment(RootComponent);
	CannonBottomMesh = CreateDefaultSubobject<UStaticMeshComponent>("Static Mesh Bottom");
	CannonBottomMesh->SetupAttachment(RootComponent);
	
	ReadyToFireWidget = CreateDefaultSubobject<UWidgetComponent>("Ready To Fire Widget");
	ReadyToFireWidget->SetupAttachment(RootComponent);
}

/* Calls method that cannon can be loaded
 */
void ACannon::OnCannonOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	SetCanCannonLoad(true, OtherActor);
}

/* Calls method that cannon can't be loaded
 */
void ACannon::OnCannonOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	SetCanCannonLoad(false, OtherActor);
}

/* Sets or unsets cannonball to be loaded. Broadcasts delegate on if cannon can be loaded or not.
 */
void ACannon::SetCanCannonLoad(bool const bSetCanLoad, AActor *CannonballToLoad)
{
	if (!bCanBeLoaded)
	{
		return;
	}
	
	if (bSetCanLoad)
	{
		CannonballToLoad = Cast<ACannonball>(CannonballToLoad);
		
		if (!IsValid(CannonballToLoad))
		{
			return;
		}
	}
	else
	{
		CannonballToLoad = nullptr;
	}
	
	const UEventBusComponent* EventBus = Cast<AGameModeLevel>(GetWorld()->GetAuthGameMode())->GetEventBusComponent();
	EventBus->CannonCanBeLoaded.Broadcast(bSetCanLoad);
}

/* Rotates and fires the cannonball at the boss. Also rotates the cannon to face at where the boss will be. Sets the
 * cannon as not ready to fire after firing.
 */
void ACannon::FireCannonball()
{
	if (!bIsCannonLoaded)
	{
		return;
	}
	
	if (!IsValid(BossEnemy))
	{
		return;
	}
	
	if (!IsValid(Cannonball))
	{
		return;
	}
	
	Cannonball->SetActorRotation(SetFiringRotation());
	Cannonball->SetAsFired();
	SetLoadable(false);
}

/* Sets whether the cannon is ready to fire. Sets if cannon is loaded, visibility of ready to fire widget, and
 * collision of cannon components based on input.
 * @param bCanFire - Whether the cannon should be ready to fire or not
 */
void ACannon::SetLoadable(bool const bCanLoad)
{
	bCanBeLoaded = bCanLoad;
	ReadyToFireWidget->SetVisibility(bCanLoad);
	
	if (bCanLoad)
	{
		CannonCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CannonballDetection->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	else
	{
		CannonCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CannonballDetection->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

// Rotate the cannon to fire at where the boss will be
FRotator ACannon::SetFiringRotation()
{
	FVector const CurrentLocation = GetActorLocation(); // Get actor location
	float const CannonballSpeed = Cannonball->GetProjectileSpeed(); // Get cannonball speed
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