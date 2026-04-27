#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameModeLevel.generated.h"

enum class EBossState : uint8;
class UWaveManagerComponent;
class AGameStateLevel;
class UEnemyManagerComponent;
class UCannonManagerComponent;
class UEventBusComponent;

UCLASS()
class WYVERNWARRIORS_API AGameModeLevel : public AGameModeBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGameModeLevel();
	
	// Triggered when the player loses the game
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Game State")
	void LoseLevel();
	
	// Triggered when the player wins the game
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Game State")
	void WinLevel();
	
	// Returns a reference to the level's game state
	UFUNCTION(BlueprintCallable, Category = "Game State")
	AGameStateLevel* GetGameStateLevelRef() const { return GameStateLevel; }
	
	// Returns a reference to the enemy management component
	UFUNCTION(BlueprintCallable, Category = "Components")
	UEnemyManagerComponent* GetEnemyManagementComponent() const { return EnemyManager; }

	// Returns a reference to the wave management component
	UFUNCTION(BlueprintCallable, Category = "Components")
	UWaveManagerComponent* GetWaveManagementComponent() const { return WaveManager; }

	// Returns a reference to the event bus
	UFUNCTION(BlueprintCallable, Category = "Components")
	UEventBusComponent* GetEventBusComponent() const { return EventBus; }

	// Returns a reference to the cannon manager
	UFUNCTION(BlueprintCallable, Category = "Components")
	UCannonManagerComponent* GetCannonManager() const { return CannonManager; }

private:
	// Set references and variables
	virtual void BeginPlay() override;
	
	// Starts the first wave.
	UFUNCTION(BlueprintCallable, Category = "Game State")
	void StartLevel() const;
	
	// Reference to the level's game state
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game State", meta = (AllowPrivateAccess = true))
	AGameStateLevel* GameStateLevel;
	
	// Wave management component for handling wave logic
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = true))
	UWaveManagerComponent* WaveManager;
	
	// Enemy management component for handling enemy spawning and behavior
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Componenets", meta = (AllowPrivateAccess = true))
	UEnemyManagerComponent* EnemyManager;
	
	// Cannon management component for handling cannons in final wave
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Componenets", meta = (AllowPrivateAccess = true))
	UCannonManagerComponent* CannonManager;
	
	// Event bus component for delegates
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Componenets", meta = (AllowPrivateAccess = true))
	UEventBusComponent* EventBus;
	
	// Name of the spawn grunt function for runtime
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Functions", meta = (AllowPrivateAccess = true))
	FString SpawnGruntFunctionName;
	
	// Calls win level when boss state is defeated.
	UFUNCTION(Category = "Game State")
	void OnBossDefeated(EBossState NewState);
	
	// Timer handle for timer before win level.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game State", meta = (AllowPrivateAccess = true))
	FTimerHandle LevelWinHandle;
	
	// Time before level win is called.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game State", meta = (AllowPrivateAccess = true))
	float LevelWinHandleTime = 5.0f;
};
