#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Cannonball.generated.h"

class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class WYVERNWARRIORS_API ACannonball : public AActor
{
	GENERATED_BODY()
public:
	// Return's the initial speed of the projectile
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetProjectileSpeed() { return ProjectileMovement->InitialSpeed; }
	
private:	
	// Sets default values for this actor's properties
	ACannonball();
	
	// Component for mesh of the actor
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Componenets", meta = (AllowPrivateAccess="true"))
	UStaticMeshComponent* StaticMesh;
	
	//Component for the movement of the actor
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Componenets", meta = (AllowPrivateAccess="true"))
	UProjectileMovementComponent* ProjectileMovement;
};
