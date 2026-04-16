#include "Cannonball.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "GameManagers/GameModeLevel.h"
#include "GameManagers/Components/EventBusComponent.h"

/* Disables tick and sets up components on actor. Sets projectile to not automatically move.
 */
ACannonball::ACannonball()
{
	PrimaryActorTick.bCanEverTick = false; 
	
	SphereComponent = CreateDefaultSubobject<USphereComponent>("SphereComponent");
	SetRootComponent(SphereComponent);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMesh->SetupAttachment(RootComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->bAutoActivate = false;
	
	PickupWidget = CreateDefaultSubobject<UWidgetComponent>("PickupWidget");
	PickupWidget->SetupAttachment(RootComponent);
	PickupWidget->SetVisibility(false);
}

/* Stores initial actor location and gets event bus to use when the cannonball is picked up by the player. Adds 
 * set pickup sphere collision function to the wyvern pickup cannonball delegate.
 */
void ACannonball::BeginPlay()
{
	Super::BeginPlay();
	InitialLocation = GetActorLocation();
	
	EventBus = Cast<AGameModeLevel>(GetWorld()->GetAuthGameMode())->GetEventBusComponent();
	EventBus->OnVillageDestroyed.AddDynamic(this, &ACannonball::OnVillageDestroyed);
}

/* Resets cannonball fire status and cannonball itself.
 * @FName DestroyedVillage - Unused tag for destroyed village.
 */
void ACannonball::OnVillageDestroyed(FName DestroyedVillage)
{
	bHasBeenFired = false;
	ResetCannonball();
}

/* Sets collisions of input sphere to none or query online
 * @param bIsEnabled - Whether the pickup sphere collision should be enabled or not
 */
void ACannonball::SetPickUpSphereCollision(bool const bIsEnabled)
{
	if (bIsEnabled)
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	else
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

/* Activates pickup widget.
 */
void ACannonball::ActivatePickUpUI() const
{
	PickupWidget->SetVisibility(true);
}

/* Deactivates pickup widget.
 */
void ACannonball::DeactivatePickUpUI() const
{
	PickupWidget->SetVisibility(false);
}

/* Returns the speed at which the cannonball was fired
 * @return float - The speed at which the cannonball was fired
 */
float ACannonball::GetProjectileSpeed() const
{
	return ProjectileMovement->GetMaxSpeed();
}

/* Sets the activeness of the cannonball, which includes visibility and collision. Stops movement if deactivating.
 * @param bIsActive - Whether the cannonball should be active or not
 */
void ACannonball::SetActiveness(bool const bIsActive)
{
	SetActorHiddenInGame(!bIsActive);
	SetActorEnableCollision(bIsActive);
	
	if (!bIsActive)
	{
		if (ProjectileMovement->IsActive())
		{
			ProjectileMovement->StopMovementImmediately();
		}
	}
}

/* Detaches cannonball from any owning actor then reset the cannonball to its initial location and stop its movement.
 * Set cannonball as not being fired.
 */
void ACannonball::ResetCannonball()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	ProjectileMovement->StopMovementImmediately(); 
	SetActorLocation(InitialLocation);
}

/* Attaches cannonball to the specified socket on the Wyvern mesh and disables pickup sphere collision
 * @param WyvernMesh - The mesh of the Wyvern to attach the cannonball to
 * @param AttachSocket - The socket on the Wyvern mesh to attach the cannonball to
 */
void ACannonball::PickUpCannonball(USkeletalMeshComponent* WyvernMesh, FName const AttachSocket)
{
	AttachToComponent(WyvernMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachSocket);
	SetPickUpSphereCollision(false);
	PickupWidget->SetVisibility(false);
}

/* Sets the cannonball as fired by rotating itself, activating its movement, and setting it as active
 * @param BossLocation - The location of the boss to rotate towards when fired
 */
void ACannonball::SetAsFired(FRotator const FiringRotation)
{
	SetActiveness(true);
	bHasBeenFired = true;
	ProjectileMovement->Velocity = FiringRotation.Vector() * ProjectileMovement->GetMaxSpeed();
	ProjectileMovement->Activate();
}
