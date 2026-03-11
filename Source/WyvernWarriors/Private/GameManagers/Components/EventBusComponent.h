#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EventBusComponent.generated.h"

class ACannonball;
class ACannon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWyvernCanPickUpCannonball, bool, bIsInRange, ACannonball*, Cannonball = nullptr); // Delegate for wyvern being able to pick up cannonball event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewWave, bool, bIsFinalWave); // Delegate for new wave event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveComplete, bool, bIsFinalWave); // Delegate for wave completion event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCannonCanBeLoaded, bool, bCanBeLoaded, ACannon*, CannonToBeLoaded); // Delegate for wave completion event

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UEventBusComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Delegate for wyvern being able to pick up cannonball
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Cannonball")
	FWyvernCanPickUpCannonball WyvernCanPickUpCannonball;

	// Delegate for when a new wave happens
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Wave")
	FOnNewWave OnNewWave;

	// Delegate for when a wave is completed
	UPROPERTY(BlueprintAssignable, Category = "Wave")
	FOnWaveComplete OnWaveComplete;
	
	// Delegate for when a cannon can be loaded or not
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Cannon")
	FCannonCanBeLoaded CannonCanBeLoaded;

private:	
	// Sets default values for this component's properties
	UEventBusComponent();
};