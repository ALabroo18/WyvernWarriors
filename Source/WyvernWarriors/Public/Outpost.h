#pragma once

#include "CoreMinimal.h"
#include "EnemyPatrolRoute.h"
#include "GameFramework/Actor.h"
#include "Outpost.generated.h"

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
	UFUNCTION(BlueprintCallable, Category = "Outpost")
	bool GetPlayerCapturedStatus() const { return bIsPlayerControlled; }; 

	// Gets the patrol route for enemies at this outpost
	UFUNCTION(BlueprintCallable, Category = "Outpost")
	AEnemyPatrolRoute* GetOutpostPatrolRoute() const { return OutpostPatrolRoute; } 

private:
	// Whether the outpost is controlled by the player	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outpost", meta = (AllowPrivateAccess = true))
	bool bIsPlayerControlled = true; 

	// The patrol route for enemies at this outpost
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Outpost", meta = (AllowPrivateAccess = true))
	AEnemyPatrolRoute* OutpostPatrolRoute; 
};
