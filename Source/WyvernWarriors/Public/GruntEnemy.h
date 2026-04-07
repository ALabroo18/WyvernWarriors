#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "GruntEnemy.generated.h"

class UEventBusComponent;
class AGruntEnemyProjectile;
class AGruntEnemyController;
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
	
	// Toggles grunt enemy to be on/off
	AGruntEnemyController* ToggleGruntEnemy(bool const bToggleActive);
	
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
	
	// Gets on route bool
	bool GetOnPatrolRoute() const { return bOnRoute; }
	
	// Move towards player character while avoiding nearby enemies
	void ChasePlayerCharacter(float const DeltaTime);
	
	// Flee from the character until far enough
	void FleePlayerCharacter(float const DeltaTime);

private:
	// Sets grunt variables on game start
	virtual void BeginPlay() override;
	
	// Event bus component used for delegates
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	UEventBusComponent* EventBus;
	
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
	
	// Whether the enemy has seen the player
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Behavior", meta = (AllowPrivateAccess = true))
	bool bSeenPlayer = false;
	
	// Whether the enemy is an egg thief
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Behavior", meta = (AllowPrivateAccess = true))
	bool bIsEggThief = false;
	
	// Projectile used for attacking player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (AllowPrivateAccess = true))
	TSubclassOf<AGruntEnemyProjectile> AttackProjectile;
	
	// Material used to highlight the enemy
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = true))
	UMaterialInterface* HighlightMaterial;
	
	// Whether the grunt is active in world or not
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Behavior", meta = (AllowPrivateAccess = true))
	bool bIsActive = false;
};
