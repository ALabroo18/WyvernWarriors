#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CannonManagerComponent.generated.h"

class ACannonball;
enum class EBossState : uint8;
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
	UFUNCTION(BlueprintCallable, Category = "Wave Event")
	void SetCannonActivatables(bool const bBecomeActive);

	// Sets up cannons and cannonball stacks for final wave
	UFUNCTION(BlueprintCallable, Category = "Wave Event")
	void OnNewWave(bool const bIsFinalWave);
	
	// Sets the starting variables for cannon management
	UFUNCTION(BlueprintCallable, Category = "Wave Event")
	void SetupCannonManager();
	
	// Sets boss reference for cannons.
	UFUNCTION(BlueprintCallable, Category = "Boss")
	void SetCannonsBoss(ABossEnemy* BossEnemy);

private:
	// Sets cannon as fireable or not depending on boss state
	UFUNCTION(BlueprintCallable, Category = "Boss")
	void EnableCannonsAndCannonballs(EBossState NewBossState);
	
	// Disables cannonball usage when cannon is loaded.
	UFUNCTION(Category = "Boss")
	void OnCannonLoaded();
	
	// Disables active cannon and cannonball usage when boss destroys village.
	UFUNCTION(Category = "Boss")
	void OnVillageDestroyed(FName DestroyedVillage);
	
	// Set a random cannon as able to fire.
	UFUNCTION(BlueprintCallable, Category = "Cannons")
	void SetCannonLoadable();
	
	// Set a random cannon as unable to fire.
	UFUNCTION(BlueprintCallable, Category = "Cannons")
	void SetCannonUnloadable() const;
	
	// Sets all cannonballs pickup sphere collision as enabled.
	UFUNCTION(Category = "Cannonballs")
	void SetCannonballsGrabbable();
	
	// Sets all cannonballs pickup sphere collision as disabled.
	UFUNCTION(Category = "Cannonballs")
	void SetCannonballsUngrabbable();
	
	// Gets the cannon closest to the boss.
	UFUNCTION(Category = "Cannons")
	ACannon* GetCannonClosestToBoss();

	// Array of cannonballs and cannonball stacks.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Cannonballs", meta = (AllowPrivateAccess = true))
	TArray<AActor*> CannonballsAndStacks;
	
	// Array of cannonballs.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Cannonballs", meta = (AllowPrivateAccess = true))
	TArray<ACannonball*> Cannonballs;
	
	// Reference to inactive cannons.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Cannons", meta = (AllowPrivateAccess = true))
	TArray<ACannon*> Cannons;
	
	// Reference to active cannon.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Cannons", meta = (AllowPrivateAccess = true))
	ACannon* ActiveCannon;
};
