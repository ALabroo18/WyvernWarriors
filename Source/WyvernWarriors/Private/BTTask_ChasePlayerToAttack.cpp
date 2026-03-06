#include "BTTask_ChasePlayerToAttack.h"
#include "GruntEnemy.h"
#include "BehaviorTree/BlackboardComponent.h"

// Sets default values of task node
UBTTask_ChasePlayerToAttack::UBTTask_ChasePlayerToAttack()
{
	NodeName = "Chase Player"; // Set name
	bNotifyTick = true; // Set as able to tick
}

// Get information of blackboard key
void UBTTask_ChasePlayerToAttack::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	UBlackboardData* BlackboardData = GetBlackboardAsset(); // Get blackboard data
	
	// Get ID and class of blackboard keys
	if (ensure(BlackboardData))
	{
		SelfGruntKey.ResolveSelectedKey(*BlackboardData);
	}
}

// Execute the task and get required values
EBTNodeResult::Type UBTTask_ChasePlayerToAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Get blackboard component
	const UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!ensure(BlackboardComponent))
	{
		return EBTNodeResult::Failed; 
	}
	
	return EBTNodeResult::InProgress; // In progress if chasing player
}

// Make enemy chase player until too close
void UBTTask_ChasePlayerToAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	
	// Get grunt self
	AGruntEnemy* GruntEnemy = Cast<AGruntEnemy>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(SelfGruntKey.SelectedKeyName));
	if (!IsValid(GruntEnemy))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	GruntEnemy->ChasePlayerCharacter(DeltaSeconds); // Make the enemy chase the player
}