#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EnemyBase.generated.h"

class AEnemyPatrolRoute;
class USplineComponent;
class ACharacter;
class USkeletalMeshComponent;
class UCapsuleComponent;
class UFloatingPawnMovement;

UCLASS()
class WYVERNWARRIORS_API AEnemyBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AEnemyBase();
	
	// Sets up values
	virtual void BeginPlay() override;
	
	// Returns the distance along the patrol route
	float GetDistanceAlongRoute() const { return DistanceAlongSpline; };
	
	// Moves the enemy along the spline path
	UFUNCTION(BlueprintCallable, Category = "Movement")
	virtual void MoveAlongSpline(float DeltaTime);
	
	// Sets grunt on patrol route if close enough
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void CheckOnPatrolRoute();
	
	// Rotates and moves the enemy with the specified rotation, away from actors if provided
	UFUNCTION(Category = "Movement")
	virtual void ReturnToRoute(float const DeltaTime);
	
	// Initializes enemy variables before spawning
	UFUNCTION(BlueprintCallable, Category = "Initialization")
	virtual void InitializeEnemy(float InitialDistance, AEnemyPatrolRoute* Route, bool bSpawnOnRoute);
	
	// Modifies the enemy's health by a specified amount
	UFUNCTION(BlueprintCallable, Category = "Health")
	virtual void ModifyCurrentHealth(float Amount);
	
	// Set's the health bar fill
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Health")
	void SetHealthBarPercent();
	
	// Abstract method for attacking the player
	UFUNCTION(BlueprintCallable, Category = "Attack")
	virtual void AttackPlayer() {};
	
	// Abstract method to destroy self
	UFUNCTION(BlueprintCallable, Category = "Health")
	virtual void DestroySelfEnemy() {};

protected:
	// Rotates and moves the enemy with the specified rotation, away from actors if provided
	UFUNCTION(Category = "Movement")	
	void RotateAndMove(FVector& Direction, const float DeltaTime, const TArray<AActor*>& ActorsToAvoid = TArray<AActor*>());
	
	// Modifies direction if there is possible collisions during movement.
	UFUNCTION(Category = "Movement")
	virtual void CheckForMovementCollision(FVector& Direction, const TArray<AActor*>& ActorsToAvoid = TArray<AActor*>()) const;
	
	// Array of enemies that are within a certain radius
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
	TArray<AActor*> NearbyEnemies;
	
	// Whether the enemy is currently moving along the patrol route
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (AllowPrivateAccess = true))
	bool bOnRoute = false;
	
	// Reference to the player character
	UPROPERTY()
	ACharacter* PlayerCharacter;
	
	// Capsule component for collision
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Componenets", meta = (AllowPrivateAccess = true))
	UCapsuleComponent* CapsuleComponent;
	
	// Skeletal mesh component for the enemy's visual representation
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Componenets", meta = (AllowPrivateAccess = true))
	USkeletalMeshComponent* SkeletalMesh;
	
	// Movement component for floating movement
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Componenets", meta = (AllowPrivateAccess = true))
	UFloatingPawnMovement* FloatingPawnMovement;
	
	// Max movement speed of the enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = true))
	float MaxMovementSpeed = 6000.f;
	
	// Distance traveled along the spline
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = true))
	float DistanceAlongSpline = 0.f; 
	
	// Patrol route for to get spline from
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = true))
	AEnemyPatrolRoute* PatrolRoute = nullptr; 
	
	// Spline component for enemy movement
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
	USplineComponent* SplineComponent = nullptr;
	
	// Max health of the enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = true))
	float MaxHealth = 1.f;
	
	// Current health of the enemy
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = true))
	float CurrentHealth = 1.f;
};
