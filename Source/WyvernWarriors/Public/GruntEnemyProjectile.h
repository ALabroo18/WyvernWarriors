// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <limits>
#include "GameFramework/Actor.h"
#include "GruntEnemyProjectile.generated.h"

class UProjectileMovementComponent;

UCLASS()
class WYVERNWARRIORS_API AGruntEnemyProjectile : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AGruntEnemyProjectile();
	
	// Checks for player collision and applies damage if possible
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void CheckForPlayerToDamage(AActor* CollidedActor);
	
	// Homes the projectile towards the player if within range and angle
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void HomeTowardsPlayer();

protected:
	// Should the projectile try to home in on targets
	bool bTryToHome = true;
	
	// Current distance to player
	float SetDistanceToPlayer = std::numeric_limits<float>::max();
	
	// Amount of damage the projectile deals
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = true))
	float DamageAmount = 25.f;
	
	// Range within which the projectile can home in on targets
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = true))
	float HomingRange = 10000.f;
	
	// Angle within which the projectile can home in on targets
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = true))
	float HomingAngle = 60.f;
	
	// Strength of the homing effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = true))
	float HomingStrength = 500.f;
	
	// Projectile movement component
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = true))
	UProjectileMovementComponent* ProjectileMovementComponent;
};
