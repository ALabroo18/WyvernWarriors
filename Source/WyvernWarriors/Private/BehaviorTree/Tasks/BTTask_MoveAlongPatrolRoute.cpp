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
	}
}

// Invoked when the task is executed
EBTNodeResult::Type UBTTask_MoveAlongPatrolRoute::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Check for blackboard component
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!ensure(BlackboardComponent))
	{
		return EBTNodeResult::Failed;
	}
	
	return EBTNodeResult::InProgress; // Task is in progress
}



// Ticks the task to update movement along the patrol route
void UBTTask_MoveAlongPatrolRoute::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// Get grunt from blackboard key
	AGruntEnemy* GruntEnemy = Cast<AGruntEnemy>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(SelfGruntKey.SelectedKeyName));
	if (!IsValid(GruntEnemy))
	{
		return;
	}
	
	GruntEnemy->MoveAlongSpline(DeltaSeconds); // Call the function to move along the spline
}