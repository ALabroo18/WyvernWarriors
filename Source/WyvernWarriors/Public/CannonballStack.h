#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CannonballStack.generated.h"

class USphereComponent;

UCLASS()
class WYVERNWARRIORS_API ACannonballStack : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets cannonball stack as active or not
	UFUNCTION(BlueprintCallable, Category = "Cannonball Stack")
	void SetActiveness(bool const bIsActive) const;
	
	// Sets cannonball stack collision
	UFUNCTION(BlueprintCallable, Category = "Cannonball Stack")
	void SetPickupCollision(bool const bHasCollision) const;
	
private:
	// Sets default values
	ACannonballStack();
	
	// Root component that handles collision
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	USphereComponent* SphereComponent;
};
