#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "BossEnemy.generated.h"

class UEventBusComponent;
class UBehaviorTree;
class AGruntEnemy;
class ACharacter;
class UMaterial;
class UNiagaraSystem;
class UStaticMeshComponent;
class AWeaponDropOff;

UENUM(BlueprintType)
enum class EBossState : uint8
{
	OnPatrolRoute UMETA(DisplayName = "On Patrol Route"),
	ApproachVillage UMETA(DisplayName = "Approaching Village"),
	Hovering UMETA(DisplayName = "Hovering over Villages"),
	ReturningToPatrolRoute UMETA(DisplayName = "Returning to Patrol Route")
};

UCLASS()
class WYVERNWARRIORS_API ABossEnemy : public AEnemyBase
{
	GENERATED_BODY()

public:
	// Generates unique lightning strike locations around the player
	TArray<FVector> GenerateLightningStrikeLocations() const;
	
	// Starts the lightning strike attack
	virtual void AttackPlayer() override;
	
	// Telegraphs the lightning strikes at the specified locations
	void TelegraphLightningStrikes();
	
	// Executes the lightning strikes at the specified locations
	void ExecuteLightningStrikes(TArray<FVector> LightningStrikeLocations);
	
	// Deals damage to force field and deactivates at 0 health
	UFUNCTION(BlueprintCallable, Category = "Force Field")
	void DestroyForceField();
	
	// Reactivates force field and restores its health
	UFUNCTION(BlueprintCallable, Category = "Force Field")
	void RestoreForceField();

private:
	// Sets up boss components
	ABossEnemy();
	
	// Sets up variables
	virtual void BeginPlay() override;
	
	// Ticks the boss behavior based on current state
	virtual void Tick( float DeltaTime ) override;
	
	// Destroys self and completes the wave
	virtual void DestroySelfEnemy() override;
	
	// Reference to the event bus
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Game Managers", meta = (AllowPrivateAccess = true))
	UEventBusComponent* EventBus;
	
	// Current state boss is in
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Behavior", meta = (AllowPrivateAccess = true))
	EBossState CurrentState = EBossState::OnPatrolRoute;
	
	// Change to approach village state
	UFUNCTION(Category = "Behavior")
	void StartApproachVillage();
	
	// Moves towards a village to hover over
	UFUNCTION(Category = "Movement")
	void ApproachVillage(float const DeltaTime);
	
	// Rotates the boss then hovers
	UFUNCTION(Category = "Movement")
	void RotateThenHover(float const DeltaTime);
	
	// Moves back to patrol route
	virtual void ReturnToRoute(float const DeltaTime) override;
	
	// Gets a valid location above a village
	UFUNCTION(Category = "Movement")
	void GetVillageLocation();
	
	// Array of weapon drop-offs for villages
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
	TArray<AWeaponDropOff*> WeaponDropOffs;
	
	// Location above village for boss to travel to
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
	FVector LocationAboveVillage;
	
	// Mesh used for force field visual
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Force Field", meta = (AllowPrivateAccess = true))
	UStaticMeshComponent* ForceField;
	
	// Timer handle for restoring force field
	UPROPERTY(BlueprintReadonly, Category = "Force Field", meta = (AllowPrivateAccess = true))
	FTimerHandle ForceFieldRestoreHandle;
	
	// Is the boss using force field
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Force Field", meta = (AllowPrivateAccess = true))
	bool bIsForceFieldActive = true;
	
	// Time it takes for the force field to restore after being depleted
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Force Field", meta = (AllowPrivateAccess = true))
	float TimeToRestoreForceField = 15.f;
	
	// Niagara effect for force field breaking
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Force Field", meta = (AllowPrivateAccess = true))
	UNiagaraSystem* ForceFieldBreak;
	
	// Niagara effect for force field restoring
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Force Field", meta = (AllowPrivateAccess = true))
	UNiagaraSystem* ForceFieldRestore;
	
	// Summons grunt enemies to attack the player
	UFUNCTION(Category = "Summons")
	void SummonGruntEnemies();
	
	// Removes grunt from array and check to change state
	UFUNCTION(Category = "Summons")
	void RemoveGruntFromArray(AGruntEnemy* DeadGrunt);
	
	// Grunt enemies summoned by boss
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Summons", meta = (AllowPrivateAccess = true))
	TArray<AGruntEnemy*> GruntSummons;
	
	// Grunt enemy class for spawning
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Summons", meta = (AllowPrivateAccess = true))
	TSubclassOf<AGruntEnemy> GruntEnemyClass;
	
	// Grunt enemies summoned by boss
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Summons", meta = (AllowPrivateAccess = true))
	int32 GruntSummonAmount = 3;
	
	// Timer handles for lightning strike attack and delay
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Lightning", meta = (AllowPrivateAccess = true))
	FTimerHandle LightningStrikeHandle;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Lightning", meta = (AllowPrivateAccess = true))
	FTimerHandle LightningStrikeDelayHandle;
	
	// Range for amount of lightnings strikes
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lightning", meta = (AllowPrivateAccess = true))
	int32 MinLightningStrikes = 5;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lightning", meta = (AllowPrivateAccess = true))
	int32 MaxLightningStrikes = 7;
	
	// Interval between lightning attacks
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lightning", meta = (AllowPrivateAccess = true))
	float LightningAttackInterval = 5.f;
	
	// Delay after telegraph before executing strikes
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lightning", meta = (AllowPrivateAccess = true))
	float LightningAttackStrikeDelay = 3.f;
	
	// Radius around player for lightning strikes
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lightning", meta = (AllowPrivateAccess = true))
	float LightningStrikePlayerRadius = 5000.f;
	
	// Minimum distance between lightning strikes
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lightning", meta = (AllowPrivateAccess = true))
	float LightningStrikeAvoidance = 2000.f; 
	
	// Damage radius of each lightning strike
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lightning", meta = (AllowPrivateAccess = true))
	float LightningStrikeDamageRadius = 200.f; 
	
	// Damage each lightning strike
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lightning", meta = (AllowPrivateAccess = true))
	float LightningStrikeDamage = 50.f;
	
	// Niagara effect for lightning strike
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lightning", meta = (AllowPrivateAccess = true))
	UNiagaraSystem* LightningStrikeEffect; 
	
	// Niagara effect for lightning telegraph
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lightning", meta = (AllowPrivateAccess = true))
	UNiagaraSystem* LightningTelegraphEffect;
	
	// Damage amount on collision
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack", meta = (AllowPrivateAccess = true))
	float CollisionDamageAmount = 75.f;
};
