#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "BossEnemy.generated.h"

class ACharacter;
class UMaterial;
class UNiagaraSystem;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EForceFieldChange : uint8
{
	Hit UMETA(DisplayName = "Hit"),
	Depleted UMETA(DisplayName = "Depleted"),
	Restored UMETA(DisplayName = "Restored")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnForceFieldChange, EForceFieldChange, ForceFieldStatus);

UCLASS()
class WYVERNWARRIORS_API ABossEnemy : public AEnemyBase
{
	GENERATED_BODY()

public:
	// Delegate for force field state change
	UPROPERTY(BlueprintAssignable, Category = "Force Field")
	FOnForceFieldChange OnForceFieldChange; 
	
	// Moves the enemy along the spline path
	virtual void MoveAlongSpline(float DeltaTime) override;
	
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
	void DamageForceField();
	
	// Reactivates force field and restores its health
	UFUNCTION(BlueprintCallable, Category = "Force Field")
	void RestoreForceField();
	
	// Get the distance along the spline in the future
	FVector GetFutureLocation(float const TimePassed) const;

private:
	//
	ABossEnemy();
	
	// Sets up variables
	virtual void BeginPlay() override;
	
	// Destroys self and completes the wave
	virtual void DestroySelfEnemy() override;
	
	// Timer handles for lightning strike attack and delay
	FTimerHandle LightningStrikeHandle;
	FTimerHandle LightningStrikeDelayHandle;
	
	// Number of lightning strikes executed
	int32 StrikesExecuted = 0;
	
	// Mesh used for force field visual
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Force Field", meta = (AllowPrivateAccess = true))
	UStaticMeshComponent* ForceField;
	
	// Is the boss using force field
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Force Field", meta = (AllowPrivateAccess = true))
	bool bIsForceFieldActive = true;
	
	// Time it takes for the force field to restore after being depleted
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Force Field", meta = (AllowPrivateAccess = true))
	float TimeToRestoreForceField = 15.f;
	
	// Current health of the force fields
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Force Field", meta = (AllowPrivateAccess = true))
	int CurrentForceFieldHealth = 3;
	
	// Max health of the force fields
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Force Field", meta = (AllowPrivateAccess = true))
	int MaxForceFieldHealth = 3;
	
	// Niagara effect for force field breaking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force Field", meta = (AllowPrivateAccess = true))
	UNiagaraSystem* ForceFieldBreak;
	
	// Niagara effect for force field restoring
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force Field", meta = (AllowPrivateAccess = true))
	UNiagaraSystem* ForceFieldRestore;
	
	// Interval between lightning attacks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (AllowPrivateAccess = true))
	float LightningAttackInterval = 15.f;
	
	// Interval between lightning attack bursts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (AllowPrivateAccess = true))
	float LightningAttackBurstInterval = 3.f;
	
	// Delay after telegraph before executing strikes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (AllowPrivateAccess = true))
	float LightningAttackStrikeDelay = 2.f;
	
	// Radius around player for lightning strikes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (AllowPrivateAccess = true))
	float LightningStrikePlayerRadius = 5000.f;
	
	// Minimum distance between lightning strikes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (AllowPrivateAccess = true))
	float LightningStrikeAvoidance = 2000.f; 
	
	// Damage radius of each lightning strike
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (AllowPrivateAccess = true))
	float LightningStrikeDamageRadius = 200.f; 
	
	// Damage each lightning strike
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (AllowPrivateAccess = true))
	float LightningStrikeDamage = 50.f;
	
	// Niagara effect for lightning strike
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (AllowPrivateAccess = true))
	UNiagaraSystem* LightningStrikeEffect; 
	
	// Niagara effect for lightning telegraph
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (AllowPrivateAccess = true))
	UNiagaraSystem* LightningTelegraphEffect;
	
	// Damage amount on collision
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (AllowPrivateAccess = true))
	float CollisionDamageAmount = 75.f;
};
