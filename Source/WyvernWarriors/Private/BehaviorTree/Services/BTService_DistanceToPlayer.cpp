#include "BehaviorTree/Services/BTService_DistanceToPlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GruntEnemy.h"

// Sets default values for service
UBTService_DistanceToPlayer::UBTService_DistanceToPlayer()
{
	NodeName = TEXT("Check Distance To Player"); // Set node name
	bNotifyBecomeRelevant = true; // Set node to use on become relevant
	bNotifyTick = true; // Set node to tick
}

// Get blackboard values when service is running
void UBTService_DistanceToPlayer::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Get blackboard component
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!ensure(BlackboardComponent))
	{
		return;
	}
	
	// Get player wyvern actor
	PlayerCharacter = Cast<AActor>(BlackboardComponent->GetValueAsObject(PlayerCharacterKey.SelectedKeyName));
}

// Tick to get distance from grunt self to player
void UBTService_DistanceToPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// Check if player is valid
	if (!IsValid(PlayerCharacter))
	{
		return;
	}
	
	// Get grunt self
	const AGruntEnemy* GruntEnemy = Cast<AGruntEnemy>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(SelfGruntKey.SelectedKeyName));
	if (!IsValid(GruntEnemy))
	{
		return;
	}
	
	// Get and set distance to the player
	OwnerComp.GetBlackboardComponent()->SetValueAsFloat(DistanceToPlayerKey.SelectedKeyName, FVector::DistSquared(GruntEnemy->GetActorLocation(), PlayerCharacter->GetActorLocation()));
}