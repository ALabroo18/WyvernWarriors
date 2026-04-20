#include "Cannon.h"
#include "Cannonball.h"
#include "BossEnemy.h" 
#include "Components/WidgetComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "GameManagers/GameModeLevel.h"
#include "GameManagers/Components/EventBusComponent.h"
#include "Kismet/GameplayStatics.h"
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
	ReadyToFireWidget->SetVisibility(false);
}

/* Calls method that cannon can be loaded
 */
void ACannon::OnCannonOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	SetCanLoadCannon(true, OtherActor);
}

/* Calls method that cannon can't be loaded
 */
void ACannon::OnCannonOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	SetCanLoadCannon(false, OtherActor);
}

/* Gets the vector distance squared between the cannon and the boss.
 * @return float - Distance to the boss squared.
 */
float ACannon::GetDistanceToBossSquared()
{
	return UKismetMathLibrary::Vector_DistanceSquared(GetActorLocation(), BossEnemy->GetActorLocation());
}

/* Sets or unsets cannonball to be loaded. Broadcasts delegate on if cannon can be loaded or not.
 */
void ACannon::SetCanLoadCannon(bool const bSetCanLoad, AActor *CannonballToLoad)
{
	if (!bCanBeLoaded)
	{
		return;
	}
	
	if (!Cast<ACannonball>(CannonballToLoad))
	{
		return;
	}
	
	const UEventBusComponent* EventBus = Cast<AGameModeLevel>(GetWorld()->GetAuthGameMode())->GetEventBusComponent();
	EventBus->CannonCanBeLoaded.Broadcast(bSetCanLoad, this);
}

/* Assigns cannonball reference then deactivates it, detached it from the player, and moves it to the cannon's location.
 * Disables cannonball detection sphere then sets cannon unable to be loaded and ready to fire.
 */
void ACannon::LoadCannon(ACannonball* CannonballToLoad)
{
	Cannonball = CannonballToLoad;
	Cannonball->SetPickUpSphereCollision(false);
	Cannonball->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Cannonball->SetActorLocation(GetActorLocation());
	Cannonball->SetActiveness(false);
	
	CannonballDetection->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetCanLoadCannon(false, CannonballToLoad);
	bCanBeLoaded = false;
	bReadyToFire = true;
}

/* Rotates the cannon to face at where the boss will be and stops village destruction. Sets the cannon as not ready to 
 * fire. Plays fuse sound effect and set timer to fire cannonball after sound effect duration.
 */
void ACannon::LightCannonFuse()
{
	if (!bReadyToFire) { return; }
	if (!IsValid(BossEnemy)) { return; }
	if (!IsValid(Cannonball)) { return; }
	
	SetUnloadable();
	BossEnemy->ClearDestroyVillageTimer();
	FRotator FireRotation = SetFiringRotation();
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), CannonFuseSFX, GetActorLocation());
	
	GetWorldTimerManager().SetTimer(
		CannonFireHandle,
		FTimerDelegate::CreateUObject(this, &ACannon::FireCannonball, FireRotation),
		CannonFuseSFX->GetDuration(),
		false);
}

/* Rotates and fires the cannonball at the boss. Plays cannon fired sound effect.
 * @param FireRotation - The rotation from the cannon location to face the boss.
 */
void ACannon::FireCannonball(FRotator const FireRotation)
{
	Cannonball->SetAsFired(SetFiringRotation());
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), CannonFiredSFX, GetActorLocation());
}

/* Sets the cannon as able to be loaded. Sets ready to fire widget as visible and the collision of the cannon
 * components as queryable.
 */
void ACannon::SetLoadable()
{
	bCanBeLoaded = true;
	ReadyToFireWidget->SetVisibility(true);
	CannonCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CannonballDetection->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	GetWorldTimerManager().SetTimer(
		FireWidgetUITimer,
		this,
		&ACannon::UpdateFireWidget,
		0.1f,
		true
	);
}

/* Sets the cannon as unable to be loaded and unready to fire. Sets ready to fire widget as invisible and the collision
 * of the cannon components as no collision.
 */
void ACannon::SetUnloadable()
{
	bCanBeLoaded = false;
	bReadyToFire = false;;
	ReadyToFireWidget->SetVisibility(false);
	CannonCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CannonballDetection->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

/* Gets rotation towards boss then rotates full cannon yaw towards boss and cannon barrel roll towards boss.
 * @return FRotator - The rotation from the cannon location to face the boss.
 */
FRotator ACannon::SetFiringRotation()
{
	FRotator const RotationTowardsBoss = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), BossEnemy->GetActorLocation()); 
	FRotator CurrentRotation = GetActorRotation();
	CurrentRotation.Yaw = RotationTowardsBoss.Yaw;
	SetActorRotation(CurrentRotation);
	
	FRotator const CurrentTopRotation = FRotator(0.0, -90.0, -RotationTowardsBoss.Pitch);
	CannonTopMesh->SetRelativeRotation(CurrentTopRotation);
	return RotationTowardsBoss;
}