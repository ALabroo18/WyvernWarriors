#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyManagerComponent.generated.h"

class AGameModeLevel;
class AGruntEnemy;
class AEnemySpawnPoint;
class AEnemyPatrolRoute;
class AOutpost;
class ABossEnemy;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WYVERNWARRIORS_API UEnemyManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Getter for the boss
	ABossEnemy* GetBossEnemy() const {return BossEnemy; }
	
	// Spawns a wave of grunt enemies based on the wave number
	UFUNCTION(BlueprintCallable, Category = "Enemy Spawning")
	void SpawnGruntEnemiesForOutpost(AOutpost* outpost, bool bIsFinalWave);
	
	// Spawns a single grunt enemy at a random spawn point
	UFUNCTION(BlueprintCallable, Category = "Enemy Spawning")
	AGruntEnemy* SpawnGruntEnemy(AEnemyPatrolRoute* SpecificPatrolRoute = nullptr);
	
	// Spawns a boss enemy
	UFUNCTION(BlueprintCallable, Category = "Enemy Spawnng")
	void SpawnBoss();
	
	// Removes a grunt enemy from management
	UFUNCTION(BlueprintCallable, Category = "Enemy Management")
	void RemoveGruntEnemy(AGruntEnemy* GruntEnemy);
	
	// Destroys all enemies (grunts, boss) that are alive
	UFUNCTION(BlueprintCallable, Category = "Enemy Management")
	void DestroyAllEnemies();
	
	// Spawns boss on final wave
	UFUNCTION(Blueprintable, Category = "Enemy Spawning")
	void OnNewWave(bool const bIsFinalWave);

private:
	// Reference to game mode
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Manager", meta = (AllowPrivateAccess = true))
	AGameModeLevel* GameModeLevel;
	
	// Time between grunt spawns during runtime
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Functions", meta = (AllowPrivateAccess = true))
	float RuntimeGruntSpawnDelay;
	
	// Array of grunt enemies managed by this component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemies", meta = (AllowPrivateAccess = true))
	TArray<AGruntEnemy*> GruntEnemies;
	
	// Reference to the boss enemy
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemies", meta = (AllowPrivateAccess = true))
	ABossEnemy* BossEnemy;
	
	// Enemy grunt reference to be spawned
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemies", meta = (AllowPrivateAccess = true))
	TSubclassOf<AGruntEnemy> GruntEnemyToSpawn;
	
	// Boss enemy reference to be spawned
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Enemies", meta = (AllowPrivateAccess = true))
	TSubclassOf<ABossEnemy> BossEnemyToSpawn;
	
	// Maximum number of enemies that can be spawned
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Spawning", meta = (AllowPrivateAccess = true))
	int32 GruntSpawnCapacity = 30;
	
	// Minimum distance from player for spawning enemies
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Spawning", meta = (AllowPrivateAccess = true))
	float MinimumPlayerSpawnDistance = 20000;
	
	// Array of enemy spawn points
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Enemy Spawning", meta = (AllowPrivateAccess = true))
	TArray<AEnemySpawnPoint*> EnemySpawnPoints;
	
	// Map of enemy patrol routes with their assigned enemies
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Enemy Spawning", meta = (AllowPrivateAccess = true))
	TArray<AEnemyPatrolRoute*> EnemyPatrolRoutes;
	
	// Patrol route assigned to the boss enemy
	UPROPERTY(EditDefaultsOnly, BLueprintReadWrite, Category = "Enemy Spawning", meta = (AllowPrivateAccess = true))
	AEnemyPatrolRoute* BossPatrolRoute;
};
