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
class UBossAnimInstance;

UENUM(BlueprintType)
enum class EBossState : uint8
{
	Entering UMETA(DisplayName = "Entering"),
	OnPatrolRoute UMETA(DisplayName = "On Patrol Route"),
	ApproachVillage UMETA(DisplayName = "Approaching Village"),
	Hovering UMETA(DisplayName = "Hovering over Villages"),
	ReturningToPatrolRoute UMETA(DisplayName = "Returning to Patrol Route"),
	Defeated UMETA(DisplayName = "Defeated"),
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
	
	// Sets up variables
	virtual void BeginPlay() override;
	
	// Ticks the boss behavior based on current state
	virtual void Tick( float DeltaTime ) override;
	
	// Destroys self and completes the wave
	virtual void DestroySelfEnemy() override;
	
	// Moves back to patrol route
	virtual void ReturnToRoute(float const DeltaTime) override;
	
	// Clears the timer for the village destruction.
	UFUNCTION(Category = "Behavior")
	void ClearDestroyVillageTimer();
	
	// Clears the timer for the village destruction.
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	float GetDestroyVillageTimerProgress() const;
	
	// Deactivated force field.
	UFUNCTION(Category = "Force Field")
	void DestroyForceField();
	
	// Plays niagara system for force field destruction.
	UFUNCTION(BlueprintCallable, Category = "Force Field")
	void DestroyForceFieldNiagara();
	
	// Reactivates force field.
	UFUNCTION(Category = "Force Field")
	void RestoreForceField();
	
	// Plays niagara system for force field restoration.
	UFUNCTION(Category = "Force Field")
	void RestoreForceFieldNiagara();

protected:
	// Play final blow QTE
	UFUNCTION(BlueprintImplementableEvent, Category = "Behavior")
	void PlayFinalBlowQTE();
	
	// Sets boss into defeated state
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	void FinalBlowQTESuccess();
	
	// Restore boss health and continue fighting the player
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	void FinalBlowQTEFailure();
	
private:
	// Sets up boss components
	ABossEnemy();
	
	// Reference to the event bus
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Game Managers", meta = (AllowPrivateAccess = true))
	UEventBusComponent* EventBus;
	
	// Current state boss is in
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Behavior", meta = (AllowPrivateAccess = true))
	EBossState CurrentState = EBossState::Entering;
	
	// Switches boss to hovering state.
	UFUNCTION(Category = "Behavior")
	void SwitchToHoveringState();
	
	// Destroys the targeted village
	UFUNCTION(Category = "Behavior")
	void DestroyVillage();
	
	// Handle for village related timers.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Behavior", meta = (AllowPrivateAccess = true))
	FTimerHandle VillageTimerHandle;
	
	// Time for destroy village timer,
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior", meta = (AllowPrivateAccess = true))
	float TimeToDestroyVillage = 30.f;
	
	// QTE for final boss strike.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior", meta = (AllowPrivateAccess = true))
	TSubclassOf<UUserWidget> FinalBlowQTE;
	
	// Camera for boss intro and roar.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custscenes", meta = (AllowPrivateAccess = true))
	ACameraActor* BossEnterCamera;
	
	// Camera for boss exit and final blow.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custscenes", meta = (AllowPrivateAccess = true))
	ACameraActor* BossExitCamera;
	
	// Distance of the boss from the cameras for cutscenes.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custscenes", meta = (AllowPrivateAccess = true))
	float BossCameraDistance = 20000.f;
	
	// Direction the boss moves in a cutscene.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custscenes", meta = (AllowPrivateAccess = true))
	FVector CutsceneMovementDirection = FVector::ZeroVector;
	
	// Location of boss for intro and final blow.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custscenes", meta = (AllowPrivateAccess = true))
	FVector BossIntroFinalBlowLocation = FVector::ZeroVector;
	
	//
	UFUNCTION(Category = "Custscenes")
	void MoveIntoIntroCutscene(float DeltaTime);
	
	// Animation instance of boss.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (AllowPrivateAccess = true))
	UBossAnimInstance* BossAnimInstance;
	
	// Array of boss roar sound effects.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = true))
	TArray<USoundBase*> BossRoarSFX;
	
	// Volume of boss roar sound effects.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = true))
	float BossRoarSFXVolume = 1.f;
	
	// Sound effect for when lightning strikes.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = true))
	USoundBase* LightningStrikeSFX;
	
	// Volume of sound effect for when lightning strikes.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = true))
	float LightningStrikeSFXVolume = 1.f;
	
	// Sound concurrency settings for the lightning strikes.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = true))
	USoundConcurrency* LightningStrikeSFXConcurrency;
	
	// Sound effect for boss intro cutscene.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = true))
	USoundBase* BossIntroRoarSFX;
	
	// Change to approach village state
	UFUNCTION(Category = "Movement")
	void StartApproachVillage();
	
	// Moves towards a village to hover over
	UFUNCTION(Category = "Movement")
	void ApproachVillage(float const DeltaTime);
	
	// Rotates the boss then hovers
	UFUNCTION(Category = "Movement")
	void RotateThenHover(float const DeltaTime);
	
	// Gets a valid location above a village
	UFUNCTION(Category = "Movement")
	void GetVillageLocation();
	
	// Array of weapon drop-offs for villages
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
	TArray<AWeaponDropOff*> WeaponDropOffs;
	
	// Weapon drop off of village being targeted
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
	AWeaponDropOff* WeaponDropOff;
	
	// Location above village for boss to hover at
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
	FVector VillageHoverLocation;
	
	// Offset from village location for boss to hover at
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
	FVector BossLocationOffset = FVector(.0f, 5000.f, 10000.f);
	
	// Rotation for boss to face village
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
	FRotator RotationTowardsVillage = FRotator::ZeroRotator;
	
	// Mesh used for force field visual
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Force Field", meta = (AllowPrivateAccess = true))
	UStaticMeshComponent* ForceField;
	
	// Timer handle for force field vfx and restoration.
	UPROPERTY(BlueprintReadonly, Category = "Force Field", meta = (AllowPrivateAccess = true))
	FTimerHandle ForceFieldHandle;
	
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
