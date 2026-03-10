#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ReturnToPatrolRoute.generated.h"

class AGruntEnemy;

UCLASS()
class WYVERNWARRIORS_API UBTTask_ReturnToPatrolRoute : public UBTTaskNode
{
	GENERATED_BODY()

protected:
	// Sets default values for this task's properties 
	UBTTask_ReturnToPatrolRoute();
	
	// Get information of blackboard key
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	
	// Invoked when the task is executed
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	// Ticks the task to update movement along the patrol route
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	// Key for On Patrol Route bool
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector OnPatrolRouteKey;
	
	// Blackboard key for the player wyvern
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector SelfGruntKey;
};
