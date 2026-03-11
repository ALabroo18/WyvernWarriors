// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "GruntEnemy.generated.h"

class AGruntEnemyProjectile;
class USplineComponent;
class UArrowComponent;
class UAIPerceptionComponent;
class USphereComponent;
class UWidgetComponent;
class UMaterialInterface;
class APlayerCameraManager;

UCLASS()
class WYVERNWARRIORS_API AGruntEnemy : public AEnemyBase
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGruntEnemy();
	
	// Sets enemy variables when spawning 
	virtual void SetVariables() override;
	
	// Initializes the enemy on the patrol route at a specific distance
	virtual void InitializeEnemy(float InitialDistance, AEnemyPatrolRoute* Route, bool bSpawnOnRoute) override;
	
	// Method to destroy self
	virtual void DestroySelfEnemy() override;
	
	// Executes the attack on the player
	virtual void AttackPlayer() override;
	
	// Makes the grunt only move along the spline
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Movement")
	void MoveAlongSplineOnly();
	
	// Orients the health bar to face the player when within a certain distance
	UFUNCTION(BlueprintCallable, Category = "Health")
	void TurnHealthBarTowardsPlayer() const;
	
	// Highlights or unhighlights the grunt enemy for visual feedback
	UFUNCTION(BlueprintCallable, Category = "Appearance")
	void HighlightGruntEnemy(bool bHighlight);

	// Rotates and moves grunt back towards patrol route spot
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ReturnToRoute(float DeltaTime);
	
	// Sets grunt on patrol route if close enough
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetOnPatrolRoute();
	
	// Gets on route bool
	bool GetOnPatrolRoute() const { return bOnRoute; }
	
	// Move towards player character while avoiding nearby enemies
	void ChasePlayerCharacter(float const DeltaTime);
	
	// Flee from the character until far enough
	void FleePlayerCharacter(float const DeltaTime);
	

private:
	// Camera manager that is attached to player
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = true))
	APlayerCameraManager* PlayerCameraManager;
	
	// Max distance at which the health bar rotates to face the player
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = true))
	float HealthBarRotationDistance = 10000.f;
	
	// Arrow component indicating the attack direction
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Componenets", meta = (AllowPrivateAccess = true))
	UArrowComponent* AttackProjectileSpawn;
	
	// Sphere component for detecting the player and other enemies
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Componenets", meta = (AllowPrivateAccess = true))
	USphereComponent* DetectionSphere;
	
	// Widget component for displaying the health bar
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = true))
	UWidgetComponent* HealthBarWidget;
	
	// Whether the enemy is currently moving along the patrol route
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = true))
	bool bOnRoute = false;
	
	// Whether the enemy has seen the player
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Behavior", meta = (AllowPrivateAccess = true))
	bool bSeenPlayer = false;
	
	// Whether the enemy is an egg thief
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Behavior", meta = (AllowPrivateAccess = true))
	bool bIsEggThief = false;
	
	// Array of enemies that are within detection sphere
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (AllowPrivateAccess = true))
	TArray<AGruntEnemy*> NearbyEnemies;
	
	// Projectile used for attacking player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (AllowPrivateAccess = true))
	TSubclassOf<AGruntEnemyProjectile> AttackProjectile;
	
	// Material used to highlight the enemy
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = true))
	UMaterialInterface* HighlightMaterial;
};
