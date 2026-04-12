#pragma once

#include "CoreMinimal.h"
#include "Cannonball.h"
#include "GameFramework/Actor.h"
#include "Cannon.generated.h"

class ACannonball;
class UWidgetComponent;
class UStaticMeshComponent;
class UBoxComponent;
class USphereComponent;
class ABossEnemy;

UCLASS()
class WYVERNWARRIORS_API ACannon : public AActor
{
	GENERATED_BODY()

public:
	// Sets the cannon as able to be loaded
	UFUNCTION(Category = "Cannon")
	void SetLoadable();
	
	// Sets the cannon as unable to be loaded
	UFUNCTION(Category = "Cannon")
	void SetUnloadable();
	
	// Get whether the cannon is ready to fire
	bool GetFirable() const { return bCanBeLoaded; } 
	
	// Fires the cannonball at the boss
	UFUNCTION(BlueprintCallable, Category = "Cannon")
	void FireCannonball();
	
	// Set's the boss enemy for the cannon
	UFUNCTION(BlueprintCallable, Category = "Cannon")
	void SetBoss(ABossEnemy* Boss) { BossEnemy = Boss; }
	
	// Gets distance squared to boss.
	UFUNCTION(BlueprintCallable, Category = "Cannon")
	float GetDistanceToBossSquared();
	
	// Change opacity of ready to fire widget.
	UFUNCTION(BlueprintImplementableEvent, Category = "Firing")
	void UpdateFireWidget() const;
	
private:
	// Sets default values for this actor's properties and binds delegates
	ACannon();
	
	// Function for when the cannonball enters the cannon's loading range
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void OnCannonOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	// Function for when the cannonball leaves the cannon's loading range
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void OnCannonOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	// Function to set cannonball and cannon loaded status
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void SetCanLoadCannon(bool const bSetCanLoad, AActor* CannonballToLoad);
	
	// Sets up cannon as able to fire cannonball
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void LoadCannon(ACannonball* CannonballToLoad);
	
	// Mesh for the cannon object
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess="true"))
	UStaticMeshComponent* CannonTopMesh;
	
	// Mesh for the cannon object
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess="true"))
	UStaticMeshComponent* CannonBottomMesh;
	
	// Root component for collision
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess="true"))
	UBoxComponent* CannonCollision;
	
	// Sphere component for detecting when cannonball is in loading range
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess="true"))
	USphereComponent* CannonballDetection;
	
	// Widget that shows a cannon ready to fire
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess="true"))
	UWidgetComponent* ReadyToFireWidget;
	
	// Handle for timer that updates fire widget opacity.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Firing", meta = (AllowPrivateAccess="true"))
	FTimerHandle FireWidgetUITimer;
	
	// Bool for if the cannon ready to fire.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Firing", meta = (AllowPrivateAccess="true"))
	bool bCanBeLoaded = false;
	
	// Is the cannon loaded with a cannonball
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Firing", meta = (AllowPrivateAccess="true"))
	bool bReadyToFire = false;
	
	// Rotate the cannon to fire at where the boss will be
	UFUNCTION(BlueprintCallable, Category = "Firing")
	FRotator SetFiringRotation();
	
	// Cannonball that is fired
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Firing", meta = (AllowPrivateAccess="true"))
	ACannonball* Cannonball;
	
	// Boss enemy that is targeted
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Firing", meta = (AllowPrivateAccess="true"))
	ABossEnemy* BossEnemy;
};
