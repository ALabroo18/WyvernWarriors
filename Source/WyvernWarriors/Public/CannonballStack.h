#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ActivatableInterface.h"
#include "CannonballStack.generated.h"

class ACannonball;
class USphereComponent;
class UEventBusComponent;

UCLASS()
class WYVERNWARRIORS_API ACannonballStack : public AActor, public IActivatableInterface
{
	GENERATED_BODY()
	
public:
	// Sets cannonball stack as active or not
	UFUNCTION(BlueprintCallable, Category = "Cannonball Stack")
	virtual void SetActiveness(bool const bIsActive) override;
	
private:
	// Sets default values
	ACannonballStack();
};
