#include "Cannonball.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "GameManagers/GameModeLevel.h"

/* Disables tick and sets up components on actor.
 * Sets projectile to not automatically move
 */
ACannonball::ACannonball()
{
	PrimaryActorTick.bCanEverTick = false; 
	
	SphereComponent = CreateDefaultSubobject<USphereComponent>("SphereComponent");
	SetRootComponent(SphereComponent);
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMesh->SetupAttachment(RootComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->bAutoActivate = false;
}

/* Stores initial actor location and gets event bus to use when the cannonball is picked up by the player. Adds 
 * set pickup sphere collision function to the wyvern pickup cannonball delegate.
 */
void ACannonball::BeginPlay()
{
	Super::BeginPlay();
	InitialLocation = GetActorLocation();
	
	EventBus = Cast<AGameModeLevel>(GetWorld()->GetAuthGameMode())->GetEventBusComponent();
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

/* Returns the speed at which the cannonball was fired
 * @return float - The speed at which the cannonball was fired
 */
float ACannonball::GetProjectileSpeed() const
{
	return ProjectileMovement->GetMaxSpeed();
}

/* Sets the activeness of the cannonball, which includes visibility, collision, and movement
 * @param bIsActive - Whether the cannonball should be active or not
 */
void ACannonball::SetActiveness(bool const bIsActive)
{
	SetActorHiddenInGame(!bIsActive);
	SetActorEnableCollision(bIsActive);
	
	if (!bIsActive && ProjectileMovement->IsActive())
	{
		ProjectileMovement->StopMovementImmediately();
	}
}

/* Detaches cannonball from Wyvern and turn pickup sphere collisions on. Then reset the cannonball to its initial
 * location, and stops its movement
 */
void ACannonball::ResetCannonball()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetPickUpSphereCollision(true);
	SetActorLocation(InitialLocation);
	ProjectileMovement->StopMovementImmediately(); 
}

/* Attaches cannonball to the specified socket on the Wyvern mesh and disables pickup sphere collision
 * @param WyvernMesh - The mesh of the Wyvern to attach the cannonball to
 * @param AttachSocket - The socket on the Wyvern mesh to attach the cannonball to
 */
void ACannonball::PickUpCannonball(USkeletalMeshComponent* WyvernMesh, FName const AttachSocket)
{
	AttachToComponent(WyvernMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachSocket);
	SetPickUpSphereCollision(false);
}

/* Sets the cannonball as fired by activating its movement and setting it as active
 */
void ACannonball::SetAsFired()
{
	SetActiveness(true);
	ProjectileMovement->Activate();
}
