#include "BTTask_FleeFromPlayer.h"
#include "GruntEnemy.h"
#include "BehaviorTree/BlackboardComponent.h"

// Sets default values of task node
UBTTask_FleeFromPlayer::UBTTask_FleeFromPlayer()
{
	NodeName = "Flee From Player"; // Set name
	bNotifyTick = true; // Set as able to tick
	bCreateNodeInstance = true; // Set node to be unique for each grunt
}

// Get information of blackboard key
void UBTTask_FleeFromPlayer::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	UBlackboardData* BlackboardData = GetBlackboardAsset(); // Get blackboard data
	
	// Get ID and class of blackboard keys
	if (ensure(BlackboardData))
	{
		SelfGruntKey.ResolveSelectedKey(*BlackboardData);
		PlayerCharacterKey.ResolveSelectedKey(*BlackboardData);
		FleeingKey.ResolveSelectedKey(*BlackboardData);
		DistanceToPlayerKey.ResolveSelectedKey(*BlackboardData);
	}
}

// Execute the task and get required values
EBTNodeResult::Type UBTTask_FleeFromPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Get blackboard component
	const UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!ensure(BlackboardComponent))
	{
		return EBTNodeResult::Failed; 
	}
	
	// Get grunt self
	GruntEnemy = Cast<AGruntEnemy>(BlackboardComponent->GetValueAsObject(SelfGruntKey.SelectedKeyName));
	if (!IsValid(GruntEnemy))
	{
		return EBTNodeResult::Failed;
	}
	
	// Get player character
	PlayerCharacter = Cast<AActor>(BlackboardComponent->GetValueAsObject(PlayerCharacterKey.SelectedKeyName));
	if (!IsValid(PlayerCharacter))
	{
		return EBTNodeResult::Failed;
	}
	
	return EBTNodeResult::InProgress; // In progress if chasing player
}

// Make enemy flee from player until far enough
void UBTTask_FleeFromPlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	
	// Check if grunt is valid
	if (!IsValid(GruntEnemy))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	// Stop fleeing if far enough from player
	if (OwnerComp.GetBlackboardComponent()->GetValueAsFloat(DistanceToPlayerKey.SelectedKeyName) > (FleeToDistance * FleeToDistance))
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(FleeingKey.SelectedKeyName, false);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
	
	GruntEnemy->FleePlayerCharacter(DeltaSeconds); // Make the enemy flee from player
}