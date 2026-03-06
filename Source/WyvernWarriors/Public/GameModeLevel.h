#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameModeLevel.generated.h"

class UWaveManagerComponent;
class AGameStateLevel;
class UEnemyManagerComponent;
class UCannonManagerComponent;

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
	AGameStateLevel* GetGameStateLevelRef() const { return GameStateLevelRef; }
	
	// Returns a reference to the enemy management component
	UFUNCTION(BlueprintCallable, Category = "Components")
	UEnemyManagerComponent* GetEnemyManagementComponent() const { return EnemyManagementComponent; }

	// Returns a reference to the wave management component
	UFUNCTION(BlueprintCallable, Category = "Components")
	UWaveManagerComponent* GetWaveManagementComponent() const { return WaveManagementComponent; }

private:
	// Reference to the level's game state
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game State", meta = (AllowPrivateAccess = true))
	AGameStateLevel* GameStateLevelRef;
	
	// Wave management component for handling wave logic
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = true))
	UWaveManagerComponent* WaveManagementComponent;
	
	// Enemy management component for handling enemy spawning and behavior
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Componenets", meta = (AllowPrivateAccess = true))
	UEnemyManagerComponent* EnemyManagementComponent;
	
	// Cannon management component for handling cannons in final wave
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Componenets", meta = (AllowPrivateAccess = true))
	UCannonManagerComponent* CannonManagementComponent;
	
	// Name of the spawn grunt function for runtime
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Functions", meta = (AllowPrivateAccess = true))
	FString SpawnGruntFunctionName;
};
