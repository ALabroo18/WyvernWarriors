#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveAlongPatrolRoute.generated.h"

class AGruntEnemy;

UCLASS()
class WYVERNWARRIORS_API UBTTask_MoveAlongPatrolRoute : public UBTTaskNode
{
	GENERATED_BODY()

public:
	// Sets default values for this task's properties
	UBTTask_MoveAlongPatrolRoute();
	
	// Get information of blackboard key
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	
	// Invoked when the task is executed
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	// Ticks the task to update movement along the patrol route
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// Blackboard key for the player wyvern
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector SelfGruntKey;
	
	// Blackboard key for whether the grunt is on the patrol route
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector OnPatrolRouteKey;
};
