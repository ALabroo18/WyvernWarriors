#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FleeFromPlayer.generated.h"

class AGruntEnemy;

UCLASS()
class WYVERNWARRIORS_API UBTTask_FleeFromPlayer : public UBTTaskNode
{
	GENERATED_BODY()
	
protected:
	// Sets default values of task node
	UBTTask_FleeFromPlayer();
	
	// Get information of blackboard key
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	
	// Execute the task and get required values
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	// Make enemy chase player until too close
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	// Distance the enemy should flee to
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	float FleeToDistance;
	
	// Blackboard key for bool for fleeing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector FleeingKey;
	
	// Blackboard key for the controlled grunt.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector SelfGruntKey;
	
	// Blackboard key for the player wyvern
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector PlayerCharacterKey;
	
	// Blackboard key for the distance to player.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector DistanceToPlayerKey;
	
	// Reference to grunt enemy self
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	AGruntEnemy* GruntEnemy;
	
	// Reference to the player
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	AActor* PlayerCharacter;
};
