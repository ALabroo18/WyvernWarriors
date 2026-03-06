#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cannon.generated.h"

class ACannonball;
class UWidgetComponent;
class UStaticMeshComponent;
class UBoxComponent;
class ABossEnemy;

UCLASS()
class WYVERNWARRIORS_API ACannon : public AActor
{
	GENERATED_BODY()

public:
	// Set's whether the cannon is ready to fire
	UFUNCTION(Category = "Cannon")
	void SetFirable(bool const bCanFire);
	
	// Get whether the cannon is ready to fire
	bool GetFirable() const { return bIsReadyToFire; } 
	
	// Fires the cannonball at the boss
	UFUNCTION(BlueprintCallable, Category = "Cannon")
	void FireCannonball();
	
	// Set's the boss enemy for the cannon
	UFUNCTION(BlueprintCallable, Category = "Cannon")
	void SetBoss(ABossEnemy* Boss) { BossEnemy = Boss; }
	
private:
	// Sets default values for this actor's properties
	ACannon();
	
	// Rotate the cannon to fire at where the boss will be
	UFUNCTION(BlueprintCallable, Category = "Firing")
	FRotator SetFiringRotation(ACannonball* Ball);
	
	// Mesh for the cannon object
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess="true"))
	UStaticMeshComponent* CannonTopMesh;
	
	// Mesh for the cannon object
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess="true"))
	UStaticMeshComponent* CannonBottomMesh;
	
	// Root component for collision
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess="true"))
	UBoxComponent* BoxComponent;
	
	// Widget that shows a cannon ready to fire
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess="true"))
	UWidgetComponent* ReadyToFireWidget;
	
	// Is the cannon ready to fire
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Firing", meta = (AllowPrivateAccess="true"))
	bool bIsReadyToFire = false;
	
	// Cannonball that is fired
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Firing", meta = (AllowPrivateAccess="true"))
	TSubclassOf<ACannonball> Cannonball;
	
	// Boss enemy that is targeted
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Firing", meta = (AllowPrivateAccess="true"))
	ABossEnemy* BossEnemy;
};
