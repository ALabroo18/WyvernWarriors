#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyPatrolRoute.generated.h"

class USplineComponent;

UCLASS()
class WYVERNWARRIORS_API AEnemyPatrolRoute : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AEnemyPatrolRoute();
	
	// Gets the enemies assigned to this patrol route
	UFUNCTION(BlueprintCallable, Category = "Patrol Route")
	int32 GetNumEnemiesOnRoute() const { return NumEnemiesOnRoute; }
	
	// Adds an enemy to the patrol route
	UFUNCTION(BlueprintCallable, Category = "Patrol Route")
	void ModifyRouteEnemyCount(bool bIncreaseCount);
	
	// Sets the maximum amount of enemies allowed on patrol route
	UFUNCTION(BlueprintCallable, Category = "Patrol Route")
	void SetMaxEnemiesOnRoute(int32 const MaxEnemies) { MaxEnemiesOnRoute = MaxEnemies; }
	
	// Gets the maximum amount of enemies allowed on patrol route 
	UFUNCTION(BlueprintCallable, Category = "Patrol Route")
	int32 GetMaxEnemiesOnRoute() const {return MaxEnemiesOnRoute; }
	
	// Sets whether the patrol route is full
	UFUNCTION(BlueprintCallable, Category = "Patrol Route")
	void SetRouteFull();
	
	// Gets whether the patrol route is full
	UFUNCTION(BlueprintCallable, Category = "Patrol Route")
	bool GetRouteFull() const { return bIsRouteFull; }
	
	// Gets the spline component defining the patrol path
	UFUNCTION(BlueprintCallable, Category = "Components")
	USplineComponent* GetSplineComponent() const { return SplineComponent; }

protected:
	// Number of enemies currently on this patrol route
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Patrol Route", meta = (AllowPrivateAccess = true))
	int32 NumEnemiesOnRoute = 0;
	
	// Maximum number of enemies allowed on this patrol route
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Patrol Route", meta = (AllowPrivateAccess = true))
	int32 MaxEnemiesOnRoute = 4;
	
	// Whether the patrol route has reached its maximum enemy capacity
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Patrol Route", meta = (AllowPrivateAccess = true))
	bool bIsRouteFull = false;
	
	// Spline component defining the patrol path
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = true))
	USplineComponent* SplineComponent;
};
