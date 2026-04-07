#include "BossEnemy.h"
#include "GameManagers/GameModeLevel.h"
#include "GameManagers/Components/WaveManagerComponent.h"
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

/* Sets up health, movement speed, and reference to player. Saves all weapon drop-offs into an array.
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
	
	// Temp; Move to event which changes boss state to move to village
	GetVillageLocation();
	// End Temp
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
	case ECurrentState::OnPatrolRoute:
		MoveAlongSpline(DeltaTime);
		break;
	case ECurrentState::GoingToVillage:
		MoveToVillage(DeltaTime);
		break;
	case ECurrentState::ReturningToPatrolRoute:
		ReturnToRoute(DeltaTime);
		break;
	case ECurrentState::Hovering:
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

/* Gets direction towards a location above a village then move and rotate towards it. Changes state to hovering and
 * exits early if close enough.
 */
void ABossEnemy::MoveToVillage(float const DeltaTime)
{
	FVector DirectionToVillage = LocationAboveVillage - GetActorLocation();

	if (UKismetMathLibrary::Vector_DistanceSquared(GetActorLocation(), LocationAboveVillage) < FMath::Square(100.f))
	{
		// Temp
		UE_LOG(LogTemp, Log, TEXT("Switching to hovering state."));
		bOnRoute = false; // Move to event that changes state to move to village
		// End Temp
		CurrentState = ECurrentState::Hovering;
		return;
	}
	
	DirectionToVillage.Normalize();
	RotateAndMove(DirectionToVillage, DeltaTime);
}

/* Rotates the boss so that it is facing straight forward. Disables tick after rotating.
 * @param DeltaTime - time since last frame.
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
		CurrentState = ECurrentState::ReturningToPatrolRoute;
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
		CurrentState = ECurrentState::OnPatrolRoute;
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

/*
 */
void ABossEnemy::SummonGruntEnemies()
{
	for (int i = 0; i < GruntSummonAmount; i++)
	{
		AGruntEnemy* SummonedGrunt = GetWorld()->SpawnActor<AGruntEnemy>(GruntEnemyClass, GetActorLocation(), FRotator::ZeroRotator);
		if (IsValid(SummonedGrunt))
		{
			SummonedGrunt->InitializeEnemy(0.f, nullptr, false);
			SummonedGrunt->GetController()->   (GruntAggressiveTree);
			GruntSummons.Add(SummonedGrunt);
		}
	}
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

