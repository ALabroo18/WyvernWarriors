#include "BTService_CheckToFlee.h"
#include "BehaviorTree/BlackboardComponent.h"

// Sets default values for service
UBTService_CheckToFlee::UBTService_CheckToFlee()
{
	NodeName = TEXT("Check To Flee"); // Set node name
	bNotifyBecomeRelevant = true; // Set node to use on become relevant
	bNotifyTick = true; // Set node to tick
}

// Get blackboard values when service is running
void UBTService_CheckToFlee::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
	FleeingKey.ResolveSelectedKey(*BlackboardData);
	DistanceToPlayerKey.ResolveSelectedKey(*BlackboardData);
}

// Tick to check if enemy should flee from player
void UBTService_CheckToFlee::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// Set enemy to flee if player is too close
	if (OwnerComp.GetBlackboardComponent()->GetValueAsFloat(DistanceToPlayerKey.SelectedKeyName) < FleeingDistance * FleeingDistance)
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(FleeingKey.SelectedKeyName, true);
	}
}
