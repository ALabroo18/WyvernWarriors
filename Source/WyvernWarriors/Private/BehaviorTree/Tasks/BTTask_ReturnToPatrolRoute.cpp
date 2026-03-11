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
		ForceReturnKey.ResolveSelectedKey(*BlackboardData);
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

/* Calls grunt enemy to return to patrol route and checks if it has reached the route. If it has, set On Patrol Route
 * to true and Force Return to false, then finish the task with success.
 */
void UBTTask_ReturnToPatrolRoute::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AGruntEnemy* GruntEnemy = Cast<AGruntEnemy>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(SelfGruntKey.SelectedKeyName));
	if (!IsValid(GruntEnemy))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	GruntEnemy->ReturnToRoute(DeltaSeconds);
	
	if (GruntEnemy->GetOnPatrolRoute())
	{
		UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
		BlackboardComponent->SetValueAsBool(OnPatrolRouteKey.SelectedKeyName, true);
		BlackboardComponent->SetValueAsBool(ForceReturnKey.SelectedKeyName, false);
		
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}