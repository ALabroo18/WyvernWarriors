#include "BossEnemy.h"
#include "GameManagers/GameModeLevel.h"
#include "GameManagers/Components/WaveManagerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SplineComponent.h"


class AEnemyPatrolRoute;

#define PLAYER_COLLISION_CHANNEL ECollisionChannel::ECC_GameTraceChannel1

/* Sets up health, movement speed, and reference to player.
 */
void ABossEnemy::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	FloatingPawnMovement->MaxSpeed = MaxMovementSpeed;
	RestoreForceField();
	
	// Move rest of boss begin play here
}

// Moves the enemy along the spline path
void ABossEnemy::MoveAlongSpline(float const DeltaTime)
{
	// If no spline component, don't move
	if (!IsValid(SplineComponent))
	{
		return;
	}
	
	DistanceAlongSpline += FloatingPawnMovement->MaxSpeed * DeltaTime; // Update distance along spline based on movement speed
	
	FRotator const SplineRotation = SplineComponent->GetRotationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World); // Get new rotation on spline
	SetActorRotation(SplineRotation); // Rotate enemy to next rotation
	
	FVector const SplineLocation = SplineComponent->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World); // Get new location on spline
	SetActorLocation(SplineLocation); // Move enemy to next location
	
	// Reset distance if end of spline reached
	if (DistanceAlongSpline >= SplineComponent->GetSplineLength())
	{
		DistanceAlongSpline -= SplineComponent->GetSplineLength();
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
		bIsForceFieldActive = false;
		
		// Remove visual of force field
		if (IsValid(SkeletalMesh))
		{
			SkeletalMesh->SetOverlayMaterial(nullptr); 
		}
		
		OnForceFieldChange.Broadcast(EForceFieldChange::Depleted); // Broadcast depletion of force field
	}
	// Broadcast hit to force field state
	else
	{
		OnForceFieldChange.Broadcast(EForceFieldChange::Hit); 
	}
}

// Reactivates force field and restores its health
void ABossEnemy::RestoreForceField()
{
	bIsForceFieldActive = true; // Set force field active
	
	// Set force field visual
	if (IsValid(SkeletalMesh))
	{
		SkeletalMesh->SetOverlayMaterial(ForceFieldMaterial);
		UE_LOG(LogTemp, Log, TEXT("Set force field"));
	}
	
	CurrentForceFieldHealth = MaxForceFieldHealth; // Set force field health to max
	OnForceFieldChange.Broadcast(EForceFieldChange::Restored); // Broadcast force field as restored
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
	if (PlayerCharacter == nullptr) return; // Early out if no player found

	StrikesExecuted = 0; // Reset strike count

	GetWorldTimerManager().SetTimer(
		LightningStrikeHandle,
		this,
		&ABossEnemy::TelegraphLightningStrikes,
		LightningAttackBurstInterval,
		StrikesExecuted < 3
	);
}

// Destroys self and completes the wave
void ABossEnemy::DestroySelfEnemy()
{
	// Get reference to the game mode
	const AGameModeLevel* GameMode = Cast<AGameModeLevel>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!IsValid(GameMode))
	{
		return;
	}
	
	// Get reference to the wave manager
	UWaveManagerComponent* WaveManagerComponent = Cast<UWaveManagerComponent>(GameMode->GetWaveManagementComponent());
	if (!IsValid(WaveManagerComponent))
	{
		return;
	}
	
	WaveManagerComponent->WaveCompleted(); // Complete the wave when defeated
	Destroy(); // Destroy self
}

// Generates unique lightning strike locations around the player
TArray<FVector> ABossEnemy::GenerateLightningStrikeLocations() const
{
	TArray<FVector> StrikeLocations;
	FVector PossibleStrikeLocation;

	int32 const NumberOfStrikes = FMath::RandRange(3, 5); // Number of lightning strikes

	// Get player location x and y
	FVector const PlayerLocation = PlayerCharacter->GetActorLocation();
	float const PlayerX = PlayerLocation.X; 
	float const PlayerY = PlayerLocation.Y;

	int32 Attempts = 0; // Counter to prevent infinite loops
	int32 ConfirmedStrikes = 0; // Counter for strikes that will be doneS

	// Generate unique strike locations
	while (ConfirmedStrikes < NumberOfStrikes)
	{
		bool bTooClose = false;
		PossibleStrikeLocation.X = FMath::RandRange(PlayerX - LightningStrikePlayerRadius, PlayerX + LightningStrikePlayerRadius); // Random x within radius
		PossibleStrikeLocation.Y = FMath::RandRange(PlayerY - LightningStrikePlayerRadius, PlayerY + LightningStrikePlayerRadius); // Random y within radius
		PossibleStrikeLocation.Z = PlayerLocation.Z; // Set z to player z

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
	StrikesExecuted++; // Increment strike count

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

