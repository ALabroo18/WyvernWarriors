#pragma once

#include "CoreMinimal.h"
#include "EnemyPatrolRoute.h"
#include "GameFramework/Actor.h"
#include "Outpost.generated.h"

class AGruntEnemy;
class AEnemyPatrolRoute;

UCLASS()
class WYVERNWARRIORS_API AOutpost : public AActor
{
	GENERATED_BODY()

public:
	// Triggered when a new wave starts
	UFUNCTION(BlueprintImplementableEvent, Category = "Wave")
	void NewWave(bool bIsFinalWave); 

	// Gets whether the outpost is controlled by the player
	UFUNCTION(BlueprintCallable, Category = "Capture")
	bool GetPlayerCapturedStatus() const { return bIsPlayerControlled; }; 
	
	// Sets whether the outpost is controlled by the player
	UFUNCTION(BlueprintCallable, Category = "Capture")
	void SetPlayerCapturedStatus(bool const bIsCaptured) { bIsPlayerControlled = bIsCaptured; }; 

	// Gets the patrol route for enemies at this outpost
	UFUNCTION(BlueprintCallable, Category = "Outpost")
	AEnemyPatrolRoute* GetOutpostPatrolRoute() const { return OutpostPatrolRoute; }
	
	// Adds a grunt enemy to the outpost's managed array of grunt enemies 
	UFUNCTION(BlueprintCallable, Category = "Enemies")
	void AddGruntEnemy(AGruntEnemy* GruntEnemy) { GruntEnemies.Add(GruntEnemy); }

private:
	// Sets initial values for variables
	virtual void BeginPlay() override;
	
	// Whether the outpost is controlled by the player	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outpost", meta = (AllowPrivateAccess = true))
	bool bIsPlayerControlled = true; 

	// The patrol route for enemies at this outpost
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Outpost", meta = (AllowPrivateAccess = true))
	AEnemyPatrolRoute* OutpostPatrolRoute;
	
	// The grunt enemies currently at this outpost
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemies", meta = (AllowPrivateAccess = true))
	TArray<AGruntEnemy*> GruntEnemies;
	
	// Maximum amount of grunts on outpost patrol route
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Enemies", meta = (AllowPrivateAccess = true))
	int32 MaxGruntOnRoute = 2;
};
