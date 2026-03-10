#include "BehaviorTree/Tasks/BTTask_MoveAlongPatrolRoute.h"
#include "GruntEnemy.h"
#include "BehaviorTree/BlackboardComponent.h"

// Sets default values for this task's properties
UBTTask_MoveAlongPatrolRoute::UBTTask_MoveAlongPatrolRoute()
{
	NodeName = "Move Along Patrol Route"; // Set name
	bNotifyTick = true; // Set as able to tick
}

// Get information of blackboard key
void UBTTask_MoveAlongPatrolRoute::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	UBlackboardData* BlackboardData = GetBlackboardAsset(); // Get blackboard data
	
	// Get ID and class of blackboard key
	if (ensure(BlackboardData))
	{
		SelfGruntKey.ResolveSelectedKey(*BlackboardData);
		OnPatrolRouteKey.ResolveSelectedKey(*BlackboardData);
	}
}

// Invoked when the task is executed
EBTNodeResult::Type UBTTask_MoveAlongPatrolRoute::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::InProgress; // Task is in progress
}



/* Moves the grunt enemy along the patrol route spline. Fails if the grunt isn't on the patrol route or if there is an
 * issue with the blackboard or grunt enemy.
 */
void UBTTask_MoveAlongPatrolRoute::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AGruntEnemy* GruntEnemy = Cast<AGruntEnemy>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(SelfGruntKey.SelectedKeyName));
	if (!IsValid(GruntEnemy))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!ensure(BlackboardComponent))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	if (!BlackboardComponent->GetValueAsBool(OnPatrolRouteKey.SelectedKeyName))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	GruntEnemy->MoveAlongSpline(DeltaSeconds);
}