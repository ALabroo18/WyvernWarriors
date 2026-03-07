#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CannonManagerComponent.generated.h"

enum class EForceFieldChange : uint8;
class ABossEnemy;
class ACannon;
class UWaveManagerComponent;
class IActivatableInterface;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WYVERNWARRIORS_API UCannonManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Set cannon activatables activeness
	UFUNCTION(BlueprintCallable, Category = "Cannons")
	void SetCannonActivatables(bool const bBecomeActive);
	
	// Sets boss enemy for cannons to target
	UFUNCTION(BlueprintCallable, Category = "Cannons")
	void SetCannonsBoss(ABossEnemy* BossEnemy);

	// Sets multiple random cannons as able to fire at the boss
	UFUNCTION(BlueprintCallable, Category = "Cannons")
	void SetMultipleCannonsFireable(bool bCanFire, int32 NumberOfCannons);

	// Returns the maximum amount of active cannons
	int32 GetMaxActiveCannons() const {return MaxActiveCannons; };

	// Sets cannon(s) as fireable or not depending on boss force field
	UFUNCTION(BlueprintCallable, Category = "Cannons")
	void ChangeCannonsFireable(EForceFieldChange const ForceFieldChange);

	// Sets up cannons and cannonball stacks for final wave
	UFUNCTION(BlueprintCallable, Category = "Cannons")
	void OnNewWave(bool const bIsFinalWave);
	
	// Sets the starting variables for cannon management
	UFUNCTION(BlueprintCallable, Category = "Cannons")
	void SetStartVariables();

private:
	// Set a random cannon as able to fire at the boss
	UFUNCTION(BlueprintCallable, Category = "Cannons")
	void SetCannonFireable(bool bCanFire);

	// Array of activatable cannon objects
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Cannons", meta = (AllowPrivateAccess = true))
	TArray<AActor*> CannonActivatables;
	
	// Reference to inactive cannons
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Cannons", meta = (AllowPrivateAccess = true))
	TArray<ACannon*> InactiveCannons;
	
	// Reference to active cannons
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Cannons", meta = (AllowPrivateAccess = true))
	TArray<ACannon*> ActiveCannons;

	// Maximum amount of active cannons at once
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Cannons", meta = (AllowPrivateAccess = true))
	int32 MaxActiveCannons = 3;
};
