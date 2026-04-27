#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EventBusComponent.generated.h"

class AGruntEnemy;
class ACannonball;
class ACannon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWyvernCanPickUpCannonball, bool, bIsInRange, ACannonball*, Cannonball = nullptr); // Delegate for wyvern being able to pick up cannonball event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewWave, bool, bIsFinalWave); // Delegate for new wave event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveComplete, bool, bIsFinalWave); // Delegate for wave completion event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCannonCanBeLoaded, bool, bCanBeLoaded, ACannon*, CannonToBeLoaded); // Delegate for wave completion event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCutsceneEvent, bool, bIsHappening); // Delegate for when cutscene happens
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGruntDeath, AGruntEnemy*, DeadGrunt); // Delegate for when grunt enemy dies
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossStateChange, EBossState, BossState); // Delegate for boss state changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnForceFieldChange, bool, bIsForceFieldActive); // Delegate for force field state change
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVillageDestroyed, FName, VillageDestroyedTag); // Delegate for when boss destroys village.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFinalBlowQTE); // Delegate for final blow.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFinalBlowFailure); // Delegate for final blow failure.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCannonLoaded); // Delegate for when cannon is loaded.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWyvernInFinalBlowRange, bool, bInRange); // Delegate for when wyvern in range for final blow.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartLevel); // Delegate for when the level starts.

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UEventBusComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Delegate for when the level starts.
	UPROPERTY(BlueprintAssignable, Category = "Level")
	FStartLevel StartLevel;
	
	// Delegate for wyvern being able to pick up cannonball.
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Cannonball")
	FWyvernCanPickUpCannonball WyvernCanPickUpCannonball;

	// Delegate for when a new wave happens.
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Wave")
	FOnNewWave OnNewWave;

	// Delegate for when a wave is completed.
	UPROPERTY(BlueprintAssignable, Category = "Wave")
	FOnWaveComplete OnWaveComplete;
	
	// Delegate for when a cannon can be loaded or not.
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Cannon")
	FCannonCanBeLoaded CannonCanBeLoaded;
	
	// Delegate for when a cannon can be loaded or not.
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Cannon")
	FOnCannonLoaded OnCannonLoaded;
	
	// Delegate for when cutscene happens.
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Cinematics")
	FCutsceneEvent CutsceneEvent;
	
	// Delegate for when grunt enemy dies.
	UPROPERTY(BlueprintAssignable, Category = "Grunt")
	FOnGruntDeath OnGruntDeath;
	
	// Delegate for the boss state change.
	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FOnBossStateChange OnBossStateChange;
	
	// Delegate for when boss destroys village.
	UPROPERTY(BlueprintAssignable, Category = "Force Field")
	FOnForceFieldChange OnForceFieldChange; 
	
	// Delegate for when boss destroys village.
	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FOnVillageDestroyed OnVillageDestroyed;
	
	// Delegate for when boss destroys village.
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Boss")
	FOnFinalBlowQTE OnFinalBlowQTE;
	
	// Delegate for when boss destroys village.
	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FOnFinalBlowFailure OnFinalBlowFailure;
	
	// Delegate for when wyvern in range for final blow.
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Boss")
	FWyvernInFinalBlowRange WyvernInFinalBlowRange;
	
private:	
	// Sets default values for this component's properties
	UEventBusComponent();
};