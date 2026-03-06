#include "BehaviorTree/Services/BTService_CheckIsAggressive.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_CheckIsAggressive::UBTService_CheckIsAggressive()
{
	NodeName = " Check Aggression"; // Set name of node
	bNotifyTick = true; // Set node to tick
	bNotifyBecomeRelevant = true; // Set node to use on become relevant
}

void UBTService_CheckIsAggressive::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Get blackboard component
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!ensure(BlackboardComponent))
	{
		return;
	}
	
	// Get blackboard data
	UBlackboardData* BlackboardData = BlackboardComponent->GetBlackboardAsset();
	if (!ensure(BlackboardData))
	{
		return;
	}
	
	// Resolve key ids and names
	AggressionKey.ResolveSelectedKey(*BlackboardData);
	DistanceToPlayerKey.ResolveSelectedKey(*BlackboardData);
}

void UBTService_CheckIsAggressive::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (OwnerComp.GetBlackboardComponent()->GetValueAsFloat(DistanceToPlayerKey.SelectedKeyName) > (AggressionDistance * AggressionDistance))
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(AggressionKey.SelectedKeyName, false);
	}
}
