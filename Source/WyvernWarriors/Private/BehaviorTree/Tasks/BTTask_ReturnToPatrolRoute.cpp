#include "BehaviorTree/Tasks/BTTask_ReturnToPatrolRoute.h"
#include "GruntEnemy.h"
#include "BehaviorTree/BlackboardComponent.h"

// Sets default values for this task's properties
UBTTask_ReturnToPatrolRoute::UBTTask_ReturnToPatrolRoute()
{
	NodeName = "Return To Patrol Route"; // Set name
	bNotifyTick = true; // Set as able to tick
}

// Get information of blackboard key
void UBTTask_ReturnToPatrolRoute::InitializeFromAsset(UBehaviorTree& Asset)
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
EBTNodeResult::Type UBTTask_ReturnToPatrolRoute::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Check for blackboard component
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent(); // Get blackboard component
	if (!ensure(BlackboardComponent))
	{
		return  EBTNodeResult::Failed;
	}
	
	return EBTNodeResult::InProgress; // Task is in progress
}

// Moves the grunt enemy back towards the patrol route
void UBTTask_ReturnToPatrolRoute::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// Get grunt from blackboard key
	AGruntEnemy* GruntEnemy = Cast<AGruntEnemy>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(SelfGruntKey.SelectedKeyName));
	if (!IsValid(GruntEnemy))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	GruntEnemy->ReturnToRoute(DeltaSeconds); // Movement and rotation of grunt to route
	
	// Succeed if grunt is on route
	if (GruntEnemy->GetOnPatrolRoute())
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(OnPatrolRouteKey.SelectedKeyName, true);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}