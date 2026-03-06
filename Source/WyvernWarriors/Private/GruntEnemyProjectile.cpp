// Fill out your copyright notice in the Description page of Project Settings.


#include "GruntEnemyProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AGruntEnemyProjectile::AGruntEnemyProjectile()
{
	// Create and configure projectile movement component
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->UpdatedComponent = RootComponent;
}

// Checks for player collision and applies damage if possible
void AGruntEnemyProjectile::CheckForPlayerToDamage(AActor* CollidedActor)
{
	// Check if Collided Actor is valid; log error and return if not
	if (!IsValid(CollidedActor))
	{
		return;
	}

	// If Collided Actor has player tag, do damage
	if (CollidedActor->ActorHasTag(FName("Player")))
	{
		UGameplayStatics::ApplyDamage(CollidedActor, DamageAmount, nullptr, this, UDamageType::StaticClass());
	}
	
	// Else if Collided Actor is an enemy (the owner), do not destroy self
	else if (CollidedActor == GetOwner())
	{
		return;
	}

	Destroy(); // Destroy projectile after collision
}

// Homes the projectile towards the player if within range and angle
void AGruntEnemyProjectile::HomeTowardsPlayer()
{
	// If not trying to home, return
	if (!bTryToHome)
	{
		return;
	}

	// Get player actor; log error and return if invalid
	const ACharacter* PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!IsValid(PlayerActor))
	{
		return;
	}

	float const CurrentDistanceToPlayer = UKismetMathLibrary::Vector_Distance(GetActorLocation(), PlayerActor->GetActorLocation()); // Distance to player

	// If distance to player is increasing, do not home
	if (CurrentDistanceToPlayer > SetDistanceToPlayer)
	{
		bTryToHome = false;
		return;
	}
	// Update distance to player
	else 
	{
		SetDistanceToPlayer = CurrentDistanceToPlayer;
	}

	// Calculate distance and direction to player
	if (CurrentDistanceToPlayer > HomingRange)
	{
		return;
	}
	
	FVector const DirectionToPlayer = (PlayerActor->GetActorLocation() - GetActorLocation()).GetSafeNormal(); // Direction vector to player
	FVector const CurrentForward = GetActorForwardVector(); // Current forward vector of projectile
	float const AngleToPlayer = FMath::RadiansToDegrees(acosf(FVector::DotProduct(CurrentForward, DirectionToPlayer))); // Angle between forward and direction to player

	if (AngleToPlayer > HomingAngle)
	{
		return;
	}

	FVector NewDirection = FMath::VInterpTo(CurrentForward, DirectionToPlayer, GetWorld()->GetDeltaSeconds(), HomingStrength / 100).GetSafeNormal(); // Interpolated new direction
	NewDirection *= GetVelocity().Size(); // Scale to current speed
	ProjectileMovementComponent->Velocity = NewDirection; // Update projectile velocity
}