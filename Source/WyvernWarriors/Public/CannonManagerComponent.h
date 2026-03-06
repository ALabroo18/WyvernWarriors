#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CannonManagerComponent.generated.h"

class ABossEnemy;
class ACannonballStack;
class ACannon;
class UWaveManagerComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WYVERNWARRIORS_API UCannonManagerComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	
	// Set cannons as visible and able to collide
	UFUNCTION(BlueprintCallable, Category = "Cannons")
	void SetCannonballsActive();
	
	// Set a random cannon as able to fire at the boss
	UFUNCTION(BlueprintCallable, Category = "Cannons")
	void SetCannonFireable(bool bCanFire);
	
	// Sets multiple random cannons as able to fire at the boss
	UFUNCTION(BlueprintCallable, Category = "Cannons")
	void SetMultipleCannonsFireable(bool bCanFire, int32 NumberOfCannons);
	
	// Reference to cannonball stacks for final wave
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Cannons", meta = (AllowPrivateAccess = true))
	TArray<ACannonballStack*> CannonballStacks;
	
	// Reference to inactive cannons
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Cannons", meta = (AllowPrivateAccess = true))
	TArray<ACannon*> InactiveCannons;
	
	// Reference to active cannons
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Cannons", meta = (AllowPrivateAccess = true))
	TArray<ACannon*> ActiveCannons;
};
