// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GruntEnemyController.generated.h"

class UAIPerceptionComponent;
class UBehaviorTree;
class UBlackboardComponent;

UCLASS()
class WYVERNWARRIORS_API AGruntEnemyController : public AAIController
{
	GENERATED_BODY()

public:
	// Set default values for this controller's properties
	AGruntEnemyController();
	
	// Called when the controller possesses a pawn
	virtual void OnPossess(APawn* InPawn) override;
	
	// Runs the aggressive behavior tree.
	UFUNCTION(Category = "AI")
	void RunAggressiveTree();

private:
	// Blackboard component for behavior tree
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = true))
	UBlackboardComponent* GruntBlackboardComponent;
	
	// Behavior tree for AI logic
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = true))
	UBehaviorTree* GruntBehaviorTree;
	
	// Aggressive behavior tree for the grunt
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = true))
	UBehaviorTree* GruntAggressiveTree;
	
	// Perception component for sensing the environment
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = true))
	UAIPerceptionComponent* GruntAIPerceptionComponent;
	
	// Key name for blackboard bool for aggressive state
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = true))
	FName AggressiveStateKey = FName("bIsAggressive");
};
