#include "BossEnemy.h"
#include "GameManagers/GameModeLevel.h"
#include "GameManagers/Components/WaveManagerComponent.h"
#include "GameManagers/Components//EnemyManagerComponent.h"
#include "GameManagers/Components//EventBusComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SplineComponent.h"
#include "WeaponDropOff.h"
#include "Kismet/KismetMathLibrary.h"
#include "GruntEnemy.h"

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
}

/* Sets up health, movement speed, and reference to player. Saves all weapon drop-offs into an array. Summons grunts
 * for first state.
 */
void ABossEnemy::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	FloatingPawnMovement->MaxSpeed = MaxMovementSpeed;
	RestoreForceField();
	
	TArray<AActor*> TempActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWeaponDropOff::StaticClass(), TempActors);
	for (AActor* Actor : TempActors)
	{
		if (IsValid(Actor))
		{
			WeaponDropOffs.Add(Cast<AWeaponDropOff>(Actor));
		}
	}
	
	const AGameModeLevel* GameModeLevel = Cast<AGameModeLevel>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!IsValid(GameModeLevel)) return;
	
	EventBus = GameModeLevel->GetEventBusComponent();
	if (!IsValid(EventBus)) return;
	
	EventBus->OnGruntDeath.AddDynamic(this, &ABossEnemy::RemoveGruntFromArray);
	
	SummonGruntEnemies();
	// Move rest of boss begin play here
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
	}
}

// Deals damage to force field and deactivates at 0 health
void ABossEnemy::DamageForceField()
{
	CurrentForceFieldHealth--; // Reduce health
	CurrentForceFieldHealth = FMath::Clamp(CurrentForceFieldHealth, 0, MaxForceFieldHealth); // Clamp health to correct values

	// Deactivate force field when health hits 0
	if (CurrentForceFieldHealth == 0)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			ForceFieldBreak,
			ForceField,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTargetIncludingScale,
			true
		);
		
		bIsForceFieldActive = false;
		ForceField->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ForceField->SetVisibility(false);
		// Temp
		UE_LOG(LogTemp, Log, TEXT("Boss Force Field Destroyed"));
		// End Temp
		OnForceFieldChange.Broadcast(EForceFieldChange::Depleted); // Broadcast depletion of force field
	}
	// Broadcast hit to force field state
	else
	{
		OnForceFieldChange.Broadcast(EForceFieldChange::Hit); 
	}
}

/* Plays niagara effect and turns force field back on. Sets force field health and bool. Broadcast force field change.
 */
void ABossEnemy::RestoreForceField()
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
	ForceField->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ForceField->SetVisibility(true);
	// Temp
	UE_LOG(LogTemp, Log, TEXT("Boss Force Field Restored"));
	// End Temp
	bIsForceFieldActive = true;
	CurrentForceFieldHealth = MaxForceFieldHealth;
	OnForceFieldChange.Broadcast(EForceFieldChange::Restored);
}

// Get the distance along the spline in the future
FVector ABossEnemy::GetFutureLocation(float const TimePassed) const
{
	float FutureSplineDistance = DistanceAlongSpline + (TimePassed * FloatingPawnMovement->MaxSpeed); // Get Distance in time passed seconds
	float const SplineLength = SplineComponent->GetSplineLength(); // Get spline length
	
	// If longer than spline length, subtract until shorter
	while (FutureSplineDistance > SplineLength)
	{
		FutureSplineDistance -= SplineLength;
	}
	
	return SplineComponent->GetLocationAtDistanceAlongSpline(FutureSplineDistance, ESplineCoordinateSpace::World); // Return the world space at that distance
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
	// Get reference to the game mode
	const AGameModeLevel* GameMode = Cast<AGameModeLevel>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!IsValid(GameMode)) return;
	
	// Get reference to the wave manager
	UWaveManagerComponent* WaveManagerComponent = Cast<UWaveManagerComponent>(GameMode->GetWaveManagementComponent());
	if (!IsValid(WaveManagerComponent)) return;
	
	WaveManagerComponent->WaveCompleted(); // Complete the wave when defeated
	Destroy(); // Destroy self
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

/* Gets direction towards a location above a village then move and rotate towards it. Changes state to hovering and
 * exits early if close enough.
 * @param DeltaTime - time since last tick.
 */
void ABossEnemy::ApproachVillage(float const DeltaTime)
{
	FVector DirectionToVillage = LocationAboveVillage - GetActorLocation();

	if (UKismetMathLibrary::Vector_DistanceSquared(GetActorLocation(), LocationAboveVillage) < FMath::Square(100.f))
	{
		UE_LOG(LogTemp, Log, TEXT("Switching to hovering state.")); // Temp
		CurrentState = EBossState::Hovering;
		EventBus->OnBossStateChange.Broadcast(CurrentState);
		return;
	}
	
	DirectionToVillage.Normalize();
	RotateAndMove(DirectionToVillage, DeltaTime);
}

/* Rotates the boss so that it is facing straight forward. Disables tick after rotating.
 * @param DeltaTime - time since last tick.
 */
void ABossEnemy::RotateThenHover(float const DeltaTime)
{
	float const YawValue = GetActorRotation().Yaw;
	float const RollValue = GetActorRotation().Roll;
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), FRotator(0.f, YawValue, RollValue), DeltaTime, .5f));
	if (GetActorRotation().Equals(FRotator(0.f, YawValue, RollValue), .5f))
	{
		UE_LOG(LogTemp, Log, TEXT("Done Rotating, now hovering"));
		SetActorTickEnabled(false);
		// Temp
		SetActorTickEnabled(true);
		CurrentState = EBossState::ReturningToPatrolRoute;
		// End Temp
	}
}

/* Call parent ReturnToRoute and change state when back on patrol route.
 */
void ABossEnemy::ReturnToRoute(float const DeltaTime)
{
	Super::ReturnToRoute(DeltaTime);
	
	if (bOnRoute)
	{
		CurrentState = EBossState::OnPatrolRoute;
		SummonGruntEnemies();
	}
}

/* Sets the location above the village to approach. Determines if location is feasible through line check before
 * setting.
 */
void ABossEnemy::GetVillageLocation()
{
	FHitResult Hit;
	FCollisionShape const MySphere = FCollisionShape::MakeSphere(500.f);
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.AddIgnoredActor(PlayerCharacter);
	
	TArray<AWeaponDropOff*> ValidWeaponDropOffs = WeaponDropOffs;
	while (!ValidWeaponDropOffs.IsEmpty())
	{
		AWeaponDropOff* WeaponDropOff = ValidWeaponDropOffs[FMath::RandRange(0, ValidWeaponDropOffs.Num() - 1)];
		ValidWeaponDropOffs.Remove(WeaponDropOff);
		
		LocationAboveVillage = WeaponDropOff->GetActorLocation() + FVector(0.f, 0.f, 15000.f);
		GetWorld()->SweepSingleByChannel(
			Hit,
			GetActorLocation(),
			LocationAboveVillage,
			FQuat::Identity,
			ECollisionChannel::ECC_WorldStatic,
			MySphere,
			CollisionParams
		);
		
		if (!Hit.bBlockingHit)
		{
			UE_LOG(LogTemp, Warning, TEXT("Something in way of village, getting new one."))
			break;
		}
	}
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
		FTransform GruntSpawnTransform = EnemyManagerComponent->GetGruntSpawnTransform(nullptr, RouteDistance);
		AGruntEnemy* SummonedGrunt = EnemyManagerComponent->SpawnGruntEnemy(
			GruntSpawnTransform,
			0,
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

	for (const FVector& StrikeLocation : LightningStrikeLocations)
	{
		// Spawn lightning effect at strike location
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			LightningTelegraphEffect,
			StrikeLocation + FVector(0.f, 0.f, 50000.f), // Start above strike location
			FRotator::ZeroRotator,
			FVector(1.f),
			true,
			true,
			ENCPoolMethod::AutoRelease
		);
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
	FHitResult HitResult; // Hit result for collision detection
	FCollisionQueryParams CollisionParams; // Collision query parameters
	CollisionParams.bTraceComplex = false; // Don't use complex collision
	CollisionParams.bReturnPhysicalMaterial = false; // No need for physical material

	// Spawn lightning effects at strike locations
	for (const FVector& StrikeLocation : LightningStrikeLocations)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			LightningStrikeEffect,
			StrikeLocation + FVector(0.f, 0.f, 50000.f), // Start above strike location
			FRotator::ZeroRotator,
			FVector(1.f),
			true,
			true,
			ENCPoolMethod::AutoRelease
		);

		bool bHit = GetWorld()->SweepSingleByChannel(
			HitResult,
			StrikeLocation + FVector(0.f, 0.f, 50000.f), // Start above strike location
			StrikeLocation * FVector(1.f, 1.f, 0.f), // End at ground level
			FQuat::Identity,
			PLAYER_COLLISION_CHANNEL,
			FCollisionShape::MakeSphere(LightningStrikeDamageRadius), // Small sphere for sweep
			CollisionParams
		);

		// Apply damage if player is hit
		if (bHit)
		{
			UGameplayStatics::ApplyDamage(
				PlayerCharacter,
				LightningStrikeDamage,
				nullptr,
				this,
				UDamageType::StaticClass()
			);
		}
	}
}

