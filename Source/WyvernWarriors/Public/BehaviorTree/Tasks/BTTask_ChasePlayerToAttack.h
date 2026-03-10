#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ChasePlayerToAttack.generated.h"

class AGruntEnemy;

UCLASS()
class WYVERNWARRIORS_API UBTTask_ChasePlayerToAttack : public UBTTaskNode
{
	GENERATED_BODY()

protected:
	// Sets default values of task node
	UBTTask_ChasePlayerToAttack();
	
	// Get information of blackboard key
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	
	// Execute the task and get required values
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	// Make enemy chase player until too close
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	// Blackboard key for the player wyvern
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector SelfGruntKey;
};
