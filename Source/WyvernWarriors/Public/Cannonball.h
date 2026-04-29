#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ActivatableInterface.h"
#include "Cannonball.generated.h"

class UWidgetComponent;
class UEventBusComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class USphereComponent;
class USkeletalMeshComponent;

UCLASS()
class WYVERNWARRIORS_API ACannonball : public AActor, public IActivatableInterface
{
	GENERATED_BODY()
public:
	// Return's the speed of the projectile
	UFUNCTION(BlueprintCallable, Category = "Cannonball")
	float GetProjectileSpeed() const;

	// Sets cannonball stack as active or not
	UFUNCTION(BlueprintCallable, Category = "Cannonball")
	virtual void SetActiveness(bool const bIsActive) override;
	
	// Resets the cannonball to its initial location and stops its movement
	UFUNCTION(BlueprintCallable, Category = "Cannonball")
	void ResetCannonball();
	
	// Attach to Wyvern and disable pickup sphere
	UFUNCTION(BlueprintCallable, Category = "Cannonball")
	void PickUpCannonball(USkeletalMeshComponent* WyvernMesh, FName const AttachSocket);
	
	// Sets cannonball active and activates projectile movement
	UFUNCTION(BlueprintCallable, Category = "Cannonball")
	void SetAsFired(FRotator const FiringRotation);
	
	// Sets collisions of input sphere to none or query online
	UFUNCTION(BlueprintCallable, Category = "Cannonball")
	void SetPickUpSphereCollision(bool const bIsEnabled);
	
	// Activates pickup widget.
	UFUNCTION(BlueprintCallable, Category = "Cannonball")
	void ActivatePickUpUI() const;
	
	// Deactivates pickup widget.
	UFUNCTION(BlueprintCallable, Category = "Cannonball")
	void DeactivatePickUpUI() const;
	
private:	
	// Sets default values for this actor's properties
	ACannonball();
	
	// Set initial activeness and get event bus
	virtual void BeginPlay() override;
	
	// Component for mesh of the actor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Componenets", meta = (AllowPrivateAccess="true"))
	UStaticMeshComponent* StaticMesh;
	
	// Component for the movement of the actor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Componenets", meta = (AllowPrivateAccess="true"))
	UProjectileMovementComponent* ProjectileMovement;

	// Root component that handles collision
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	USphereComponent* SphereComponent;
	
	// Widget that displays cannonball pickup UI
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	UWidgetComponent* PickupWidget;

	// Event bus for wyvern pickup delegate
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Delegate", meta = (AllowPrivateAccess = true));
	UEventBusComponent* EventBus;
	
	// Resets cannonball when village is destroyed.
	UFUNCTION(Category = "Cannonball")
	void OnVillageDestroyed(FName DestroyedVillage);
	
	// Initial location of the cannonball for resetting after use
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cannonball", meta = (AllowPrivateAccess = true))
	FVector InitialLocation;
	
	// Bool for if cannonball has been fired
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Cannonball", meta = (AllowPrivateAccess = true))
	bool bHasBeenFired = false;
};
