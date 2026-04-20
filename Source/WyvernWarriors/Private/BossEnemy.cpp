#include "BossEnemy.h"
#include "GameManagers/GameModeLevel.h"
#include "GameManagers/Components//EnemyManagerComponent.h"
#include "GameManagers/Components//EventBusComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "NiagaraFunctionLibrary.h"
#include "WeaponDropOff.h"
#include "Kismet/KismetMathLibrary.h"
#include "GruntEnemy.h"
#include "Blueprint/UserWidget.h"
#include "EnemyPatrolRoute.h"
#include "NiagaraComponent.h"
#include "BossAnimInstance.h"

class AEnemyPatrolRoute;

#define PLAYER_COLLISION_CHANNEL ECollisionChannel::ECC_GameTraceChannel1

/* Sets tick to true. Sets up force field component and its tick
 */
ABossEnemy::ABossEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	
	ForceField = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ForceField"));
	ForceField->SetupAttachment(SkeletalMesh);
	ForceField->bAllowConcurrentTick = false;
	ForceField->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ForceField->SetVisibility(true);
}

/* Sets up health, movement speed, and reference to event bus. Restores force field and binds function to grunt death.
 * Saves all weapon drop-offs into an array. Summons grunts for first state.
 */
void ABossEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	const AGameModeLevel* GameModeLevel = Cast<AGameModeLevel>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!IsValid(GameModeLevel)) return;
	EventBus = GameModeLevel->GetEventBusComponent();
	if (!IsValid(EventBus)) return;
	
	CurrentHealth = MaxHealth;
	FloatingPawnMovement->MaxSpeed = MaxMovementSpeed;
	EventBus->OnGruntDeath.AddDynamic(this, &ABossEnemy::RemoveGruntFromArray);
	BossAnimInstance = Cast<UBossAnimInstance>(SkeletalMesh->GetAnimInstance());
	
	TArray<AActor*> TempActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWeaponDropOff::StaticClass(), TempActors);
	for (AActor* Actor : TempActors)
	{
		if (IsValid(Actor))
		{
			WeaponDropOffs.Add(Cast<AWeaponDropOff>(Actor));
		}
	}

	SummonGruntEnemies();
}

/* Depending on boss state, move along patrol route, move above a village, hover above the villages, or move back to
 * patrol route.
 * @param DeltaTime - time since last frame.
 */
void ABossEnemy::Tick(float const DeltaTime)
{
	Super::Tick(DeltaTime);
	
	switch (CurrentState)
	{
	case EBossState::OnPatrolRoute:
		MoveAlongSpline(DeltaTime);
		break;
	case EBossState::ApproachVillage:
		ApproachVillage(DeltaTime);
		break;
	case EBossState::ReturningToPatrolRoute:
		ReturnToRoute(DeltaTime);
		break;
	case EBossState::Hovering:
		RotateThenHover(DeltaTime);
		break;
	case EBossState::Defeated:
		RotateAndMove(RetreatDirection, DeltaTime);
		break;
	}
}

/* Spawn force field break niagara system then set force field inactive in collision and visibility. Broadcast force
 * field change. Set timer to restore force field.
 */
void ABossEnemy::DestroyForceField()
{
	bIsForceFieldActive = false;
	ForceField->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ForceField->SetVisibility(false);
	UE_LOG(LogTemp, Log, TEXT("Boss Force Field Destroyed"));
	EventBus->OnForceFieldChange.Broadcast(false); 
	
	GetWorldTimerManager().SetTimer(
		ForceFieldHandle,
		this,
		&ABossEnemy::RestoreForceFieldNiagara,
		TimeToRestoreForceField,
		false
	);
}

/* Stop lightning strikes and play the force field destroyed niagara effect. Play the dazed animation. Set timer
 * to finish destroying force field.
 */
void ABossEnemy::DestroyForceFieldNiagara()
{
	GetWorldTimerManager().ClearTimer(LightningStrikeHandle);
	
	UNiagaraFunctionLibrary::SpawnSystemAttached(
		ForceFieldBreak,
		ForceField,
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::SnapToTargetIncludingScale,
		true
	);
	
	BossAnimInstance->PlayDazedAnimation();
	
	GetWorldTimerManager().SetTimer(
		ForceFieldHandle,
		this,
		&ABossEnemy::DestroyForceField,
		0.7f,
		false
		);
}

/* Sets force field active with collision and visibility. Broadcast force field change. Enable actor tick and change
 * state to return to patrol route.
 */
void ABossEnemy::RestoreForceField()
{
	ForceField->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ForceField->SetVisibility(true);
	bIsForceFieldActive = true;
	EventBus->OnForceFieldChange.Broadcast(true);
	
	SetActorTickEnabled(true);
	CurrentState = EBossState::ReturningToPatrolRoute;
	UE_LOG(LogTemp, Log, TEXT("Boss Force Field Restored"));
}

/* Plays force field restored niagara effect and stops dazed animation. Set timer to finish restoring force field.
 */
void ABossEnemy::RestoreForceFieldNiagara()
{
	UNiagaraFunctionLibrary::SpawnSystemAttached(
			ForceFieldRestore,
			ForceField,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTargetIncludingScale,
			true
		);

	BossAnimInstance->StopDazedAnimation();
	
	GetWorldTimerManager().SetTimer(
		ForceFieldHandle,
		this,
		&ABossEnemy::RestoreForceField,
		2.8f,
		false
		);
}

/* Broadcast final blow as success. Enable actor tick and get retreat direction away from patrol route. Change state
 * to defeated and broadcast change.
 */
void ABossEnemy::FinalBlowQTESuccess()
{
	EventBus->OnFinalBlowQTE.Broadcast(false);
	SetActorTickEnabled(true);
	RetreatDirection.X = GetActorLocation().X - PatrolRoute->GetActorLocation().X;
	RetreatDirection.Y = GetActorLocation().Y - PatrolRoute->GetActorLocation().Y;
	RetreatDirection.Z = PatrolRoute->GetActorLocation().Z;
	RetreatDirection.Normalize();
	CurrentState = EBossState::Defeated;
	EventBus->OnBossStateChange.Broadcast(CurrentState);
}

/* Broadcast final blow as a fail. Set health to 10% of max and restore force field.
 */
void ABossEnemy::FinalBlowQTEFailure()
{
	EventBus->OnFinalBlowQTE.Broadcast(false);
	CurrentHealth = MaxHealth/10;
	RestoreForceField();
}

// Performs a lightning strike attack on the player
void ABossEnemy::AttackPlayer()
{
	if (!IsValid(PlayerCharacter)) return; // Early out if no player found

	GetWorldTimerManager().SetTimer(
		LightningStrikeHandle,
		this,
		&ABossEnemy::TelegraphLightningStrikes,
		LightningAttackInterval,
		true
	);
}

// Destroys self and completes the wave
void ABossEnemy::DestroySelfEnemy()
{
	if (!IsValid(FinalBlowQTE)) { UE_LOG(LogTemp, Log, TEXT("Boss does not have a final blow QTE assigned.")) return; }
	
	GetWorldTimerManager().ClearTimer(ForceFieldHandle);
	EventBus->OnFinalBlowQTE.Broadcast(true);
	PlayFinalBlowQTE();
}

/* Changes state to hovering and gets rotation to look at village with 0 pitch. Starts timer to destroy village.
 */
void ABossEnemy::SwitchToHoveringState()
{
	UE_LOG(LogTemp, Log, TEXT("Switching to hovering state."));
	RotationTowardsVillage = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), VillageHoverLocation - BossLocationOffset);
	RotationTowardsVillage = FRotator(0.f, RotationTowardsVillage.Yaw, RotationTowardsVillage.Roll);
	
	CurrentState = EBossState::Hovering;
	EventBus->OnBossStateChange.Broadcast(CurrentState);
	
	GetWorldTimerManager().SetTimer(
		VillageTimerHandle,
		this,
		&ABossEnemy::DestroyVillage,
		TimeToDestroyVillage,
		false);
}

/* Gets tag of targeted village weapon drop-off and broadcasts destroyed village with that tag. Removes weapon drop-off
 * from array. If no weapon drop-offs left, call lose level in game mode.
 */
void ABossEnemy::DestroyVillage()
{
	FName const DestroyedTag = WeaponDropOff->Tags.Last();
	EventBus->OnVillageDestroyed.Broadcast(DestroyedTag);
	UE_LOG(LogTemp, Log, TEXT("Boss has destroyed village with tag: %s"), *DestroyedTag.ToString());
	WeaponDropOffs.Remove(WeaponDropOff);
	if (WeaponDropOffs.IsEmpty())
	{
		AGameModeLevel* GameModeLevel = Cast<AGameModeLevel>(GetWorld()->GetAuthGameMode());
		GameModeLevel->LoseLevel();
		return;
	}
	
	GetWorldTimerManager().ClearTimer(LightningStrikeHandle);
	SetActorTickEnabled(true);
	CurrentState = EBossState::ReturningToPatrolRoute;
}

/* Clears the timer for the village destruction.
 */
void ABossEnemy::ClearDestroyVillageTimer()
{
	GetWorldTimerManager().ClearTimer(VillageTimerHandle);
}

/* Gets percentage of destroy village timer that has elapsed.
 * @return float - float percent of elapsed time of destroy village timer.
 */
float ABossEnemy::GetDestroyVillageTimerProgress() const
{
	return GetWorldTimerManager().GetTimerElapsed(VillageTimerHandle) / TimeToDestroyVillage;
}

/* Remove the dead grunt from the grunt array if referenced in it. Get village to approach and change state to approach
 * village if no grunts left in array.
 * @param DeadGrunt - reference to grunt that was destroyed.
 */
void ABossEnemy::RemoveGruntFromArray(AGruntEnemy* DeadGrunt)
{
	if (GruntSummons.Contains(DeadGrunt))
	{
		GruntSummons.Remove(DeadGrunt);
		
		if (GruntSummons.IsEmpty())
		{
			StartApproachVillage();
		}
	}
}

/* Get village to approach and change state to approach village state. Broadcast the state change then start attacking
 * the player.
 */
void ABossEnemy::StartApproachVillage()
{
	GetVillageLocation();
	CurrentState = EBossState::ApproachVillage;
	bOnRoute = false;
	AttackPlayer();
}

/* Gets direction towards a location above a village then move and rotate towards it. Switch to hovering state if close
 * enough.
 * @param DeltaTime - time since last tick.
 */
void ABossEnemy::ApproachVillage(float const DeltaTime)
{
	FVector DirectionToVillage = VillageHoverLocation - GetActorLocation();

	if (UKismetMathLibrary::Vector_DistanceSquared(GetActorLocation(), VillageHoverLocation) < FMath::Square(1000.f))
	{
		SwitchToHoveringState();
		return;
	}
	
	DirectionToVillage.Normalize();
	RotateAndMove(DirectionToVillage, DeltaTime);
}

/* Rotates the boss so that it is facing the village. Disables tick after rotating.
 * @param DeltaTime - time since last tick.
 */
void ABossEnemy::RotateThenHover(float const DeltaTime)
{
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), RotationTowardsVillage, DeltaTime, .5f));
	if (GetActorRotation().Equals(RotationTowardsVillage, .5f))
	{
		UE_LOG(LogTemp, Log, TEXT("Done Rotating, now hovering"));
		SetActorTickEnabled(false);
	}
}

/* Call parent ReturnToRoute and change state when back on patrol route.
 * @param DeltaTime - time since last frame.
 */
void ABossEnemy::ReturnToRoute(float const DeltaTime)
{
	Super::ReturnToRoute(DeltaTime);

	if (bOnRoute)
	{
		UE_LOG(LogTemp, Log, TEXT("Boss returned to route, switching to on patrol route state."));
		CurrentState = EBossState::OnPatrolRoute;
		SummonGruntEnemies();
	}
}

/* Sets the location above the village to hover at. Determines if location is feasible through line check before
 * setting. If no possible locations, try again after a second.
 */
void ABossEnemy::GetVillageLocation()
{
	FHitResult Hit;
	FCollisionShape const MySphere = FCollisionShape::MakeSphere(3000.f);
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.AddIgnoredActor(PlayerCharacter);
	
	TArray<AWeaponDropOff*> ValidWeaponDropOffs = WeaponDropOffs;
	if (ValidWeaponDropOffs.IsEmpty()) { UE_LOG(LogTemp, Warning, TEXT("There are no weapon drop-offs in the level for the boss to hover over.")); return; }
	
	while (true)
	{
		if (ValidWeaponDropOffs.IsEmpty()) { break; }
		WeaponDropOff = ValidWeaponDropOffs[FMath::RandRange(0, ValidWeaponDropOffs.Num() - 1)];
		ValidWeaponDropOffs.Remove(WeaponDropOff);
		
		VillageHoverLocation = WeaponDropOff->GetActorLocation() + BossLocationOffset;
		GetWorld()->SweepSingleByChannel(
			Hit,
			GetActorLocation(),
			VillageHoverLocation,
			FQuat::Identity,
			ECollisionChannel::ECC_WorldStatic,
			MySphere,
			CollisionParams
		);
		
		if (!Hit.bBlockingHit) { return; }
		UE_LOG(LogTemp, Warning, TEXT("Something in way of village, getting new one."));
	}
	
	GetWorldTimerManager().SetTimer(
		VillageTimerHandle,
		this,
		&ABossEnemy::GetVillageLocation,
		1.f,
		false);
}

/* Summons grunt enemies based on summon amount and makes them be only aggressive. Adds grunts to array to track.
 */
void ABossEnemy::SummonGruntEnemies()
{
	const AGameModeLevel* GameModeLevel = Cast<AGameModeLevel>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!IsValid(GameModeLevel))
	{
		UE_LOG(LogTemp, Error, TEXT("Incorrect game mode set for level to use boss."))
		return;
	}
	
	UEnemyManagerComponent* EnemyManagerComponent = GameModeLevel->GetEnemyManagementComponent();
	if (!IsValid(EnemyManagerComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("Incorrect enemy manager to use boss."))
		return;
	}
	
	for (int i = 0; i < GruntSummonAmount; i++)
	{
		float RouteDistance;
		FTransform GruntSpawnTransform = EnemyManagerComponent->GetGruntSpawnTransform(PatrolRoute, RouteDistance);
		AGruntEnemy* SummonedGrunt = EnemyManagerComponent->SpawnGruntEnemy(
			GruntSpawnTransform,
			RouteDistance,
			PatrolRoute,
			false,
			false
			);
		if (IsValid(SummonedGrunt))
		{
			UE_LOG(LogTemp, Log, TEXT("Summoned grunt enemy for boss: %s"), *SummonedGrunt->GetName())
			SummonedGrunt->UseAggressiveTreeOnly();
			GruntSummons.Add(SummonedGrunt);
		}
	}
}

// Generates unique lightning strike locations around the player
TArray<FVector> ABossEnemy::GenerateLightningStrikeLocations() const
{
	TArray<FVector> StrikeLocations;
	FVector PossibleStrikeLocation;

	int32 const NumberOfStrikes = FMath::RandRange(MinLightningStrikes, MaxLightningStrikes); // Number of lightning strikes

	// Get player location x and y
	FVector const FuturePlayerLocation = PlayerCharacter->GetActorLocation() + (PlayerCharacter->GetVelocity() * LightningAttackStrikeDelay);
	float const PlayerX = FuturePlayerLocation.X; 
	float const PlayerY = FuturePlayerLocation.Y;

	int32 Attempts = 0; // Counter to prevent infinite loops
	int32 ConfirmedStrikes = 0; // Counter for strikes that will be doneS
	
	StrikeLocations.Add(FuturePlayerLocation); // First strike is always directly on player

	// Generate unique strike locations
	while (ConfirmedStrikes < NumberOfStrikes)
	{
		bool bTooClose = false;
		PossibleStrikeLocation.X = FMath::RandRange(PlayerX - LightningStrikePlayerRadius, PlayerX + LightningStrikePlayerRadius); // Random x within radius
		PossibleStrikeLocation.Y = FMath::RandRange(PlayerY - LightningStrikePlayerRadius, PlayerY + LightningStrikePlayerRadius); // Random y within radius
		PossibleStrikeLocation.Z = FuturePlayerLocation.Z; // Set z to player z

		for (const FVector& ExistingStrikeLocation : StrikeLocations)
		{
			// Check distance to existing strike locations
			if (FVector::Dist2D(PossibleStrikeLocation, ExistingStrikeLocation) < LightningStrikeAvoidance)
			{
				Attempts++; // Increment attempt counter

				bTooClose = true; // Mark as too close

				break; // Too close to existing strike, try again
			}
		}

		if (Attempts > 99) break; // Break if too many attempts

		if (bTooClose) continue; // Retry if too close to existing strike

		StrikeLocations.Add(PossibleStrikeLocation); // Add strike location to array
		ConfirmedStrikes++; // Increase amount of confirmed strikes
	}

	return StrikeLocations;
}

// Telegraphs the lightning strikes at the specified locations
void ABossEnemy::TelegraphLightningStrikes()
{
	TArray<FVector> LightningStrikeLocations = GenerateLightningStrikeLocations(); // Get strike locations

	USoundBase* RandomBossRoarSFX = BossRoarSFX[FMath::RandRange(0, BossRoarSFX.Num() - 1)];
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), RandomBossRoarSFX, GetActorLocation(), BossRoarSFXVolume);
	
	for (const FVector& StrikeLocation : LightningStrikeLocations)
	{
		// Spawn lightning effect at strike location
		UNiagaraComponent* LightningStrike = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			LightningTelegraphEffect,
			StrikeLocation + FVector(0.f, 0.f, 50000.f), // Start above strike location
			FRotator::ZeroRotator,
			FVector(1.f),
			true,
			true,
			ENCPoolMethod::AutoRelease
		);
		
		LightningStrike->SetVariableFloat(FName("User.BeamWidth"), LightningStrikeDamageRadius);
	}
	
	// Set timer to execute lightning strikes after delay
	GetWorldTimerManager().SetTimer(
		LightningStrikeDelayHandle,
		FTimerDelegate::CreateUObject(this, &ABossEnemy::ExecuteLightningStrikes, LightningStrikeLocations),
		LightningAttackStrikeDelay,
		false
	);
}

// Executes the lightning strikes at the specified locations
void ABossEnemy::ExecuteLightningStrikes(TArray<FVector> LightningStrikeLocations)
{
	TArray<FHitResult> HitResults; // Hit result for collision detection
	FCollisionQueryParams CollisionParams; // Collision query parameters
	CollisionParams.bTraceComplex = false; // Don't use complex collision
	CollisionParams.bReturnPhysicalMaterial = false; // No need for physical material

	// Spawn lightning effects at strike locations
	for (const FVector& StrikeLocation : LightningStrikeLocations)
	{
		bool const bHit = GetWorld()->SweepMultiByChannel(
			HitResults,
			StrikeLocation + FVector(0.f, 0.f, 50000.f), // Start above strike location
			StrikeLocation * FVector(1.f, 1.f, 0.f), // End at ground level
			FQuat::Identity,
			PLAYER_COLLISION_CHANNEL,
			FCollisionShape::MakeSphere(LightningStrikeDamageRadius), // Small sphere for sweep
			CollisionParams
		);
		
		UNiagaraComponent* LightningStrike = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			LightningStrikeEffect,
			StrikeLocation + FVector(0.f, 0.f, 50000.f), // Start above strike location
			FRotator::ZeroRotator,
			FVector(1.f),
			true,
			false,
			ENCPoolMethod::AutoRelease
		);
		
		LightningStrike->SetVariableFloat(FName("User.BeamWidth"), LightningStrikeDamageRadius);
		
		if (bHit)
		{
			LightningStrike->SetVariableVec3(FName("User.BeamEnd"), HitResults.Last().Location - FVector(.0f, .0f, LightningStrikeDamageRadius));
			
			for (const FHitResult& HitResult : HitResults)
			{
				if (HitResult.GetActor() == PlayerCharacter)
				{
					UGameplayStatics::ApplyDamage(
						PlayerCharacter,
						LightningStrikeDamage,
						nullptr,
						this,
						UDamageType::StaticClass()
					);
					
					break;
				}
			}
		}
		else
		{
			FVector const LightningEndLocation = FVector(StrikeLocation.X, StrikeLocation.Y, -50000.f);
			LightningStrike->SetVariableVec3(FName("User.BeamEnd"), LightningEndLocation);
		}
		
		LightningStrike->Activate(true);
		UGameplayStatics::PlaySound2D(GetWorld(), LightningStrikeSFX, LightningStrikeSFXVolume, 1.f, 0.f, LightningStrikeSFXConcurrency);
	}
}

