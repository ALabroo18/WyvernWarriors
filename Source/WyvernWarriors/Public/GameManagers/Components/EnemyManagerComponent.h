#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyManagerComponent.generated.h"

class AGruntEnemyController;
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
	// Sets up enemy manager
	void SetupEnemyManager();
	
	// Getter for the boss
	ABossEnemy* GetBossEnemy() const {return BossEnemy; }
	
	// Spawns a wave of grunt enemies based on the wave number
	UFUNCTION(BlueprintCallable, Category = "Enemy Spawning")
	void SpawnGruntEnemiesForOutpost(AOutpost* Outpost, bool const bIsFinalWave);
	
	// Gets the spawn transform for a grunt
	UFUNCTION(BlueprintCallable, Category = "Enemy Spawning")
	FTransform GetGruntSpawnTransform(AEnemyPatrolRoute* SpecificPatrolRoute, float& DistanceAlongSpline);
	
	// Gets a route for the enemy to spawn on
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Enemy Spawning")
	AEnemyPatrolRoute* GetSpawnPatrolRoute();
	
	// Spawns a single grunt enemy at a random spawn point
	UFUNCTION(BlueprintCallable, Category = "Enemy Spawning")
	AGruntEnemy* SpawnGruntEnemy(const FTransform& SpawnTransform, float const DistanceAlongSpline, AEnemyPatrolRoute* Route, bool const bSpawnOnRoute, bool const bIgnoreSpawnCap);
	
	// Spawns a boss enemy
	UFUNCTION(BlueprintCallable, Category = "Enemy Spawnng")
	void SpawnBoss();
	
	// Removes a grunt enemy from the active grunt array
	UFUNCTION(BlueprintCallable, Category = "Enemy Management")
	void RemoveActiveGruntEnemy(AGruntEnemy* GruntEnemy);
	
	// Destroys all enemies (grunts, boss) that are alive
	UFUNCTION(BlueprintCallable, Category = "Enemy Management")
	void DestroyAllEnemies(bool const bIncludeBoss);
	
	// Spawns boss on final wave
	UFUNCTION(Blueprintable, Category = "Enemy Spawning")
	void OnNewWave(bool const bIsFinalWave);
	
	// Adds a grunt into the inactive grunt queue
	void AddInactiveGruntEnemy(AGruntEnemy* GruntEnemy) { InactiveGruntEnemies.Enqueue(GruntEnemy); }

	// Adds a  controller into the inactive grunt controller queue
	void AddInactiveGruntEnemyController(AGruntEnemyController* GruntEnemyController) { InactiveGruntEnemyControllers.Enqueue(GruntEnemyController); }

private:
	// Time between grunt spawns during runtime
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Functions", meta = (AllowPrivateAccess = true))
	float RuntimeGruntSpawnDelay;
	
	// Array of active grunt enemies 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemies", meta = (AllowPrivateAccess = true))
	TArray<AGruntEnemy*> ActiveGruntEnemies;
	
	// Queue of inactive grunt enemies
	TQueue<AGruntEnemy*> InactiveGruntEnemies;
	
	// Queue of inactive grunt enemy controllers
	TQueue<AGruntEnemyController*> InactiveGruntEnemyControllers;
	
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
	float MinimumPlayerSpawnDistance = 20000.f;
	
	// Minimum distance from nearest enemy for spawning enemies
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Spawning", meta = (AllowPrivateAccess = true))
	float MinimumEnemySpawnDistance = 3000.f;
	
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
