// Fill out your copyright notice in the Description page of Project Settings.


#include "Old/BTT_IncrementPatrolIndex.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SplineComponent.h"

EBTNodeResult::Type UBTT_IncrementPatrolIndex::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Get the blackboard component
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}
	
	// Get the current patrol index
	int32 CurrentIndex = BlackboardComp->GetValueAsInt("PatrolIndex");
	CurrentIndex++;
	
	// Get the patrol route
	AActor* PatrolRoute = Cast<AActor>(BlackboardComp->GetValueAsObject("PatrolRoute"));
	if (!PatrolRoute)
	{
		return EBTNodeResult::Failed;
	}

	// Get the spline component from the patrol route actor
	USplineComponent* SplineComp = PatrolRoute->FindComponentByClass<USplineComponent>();
	if (!SplineComp)
	{
		return EBTNodeResult::Failed;
	}

	const int32 TotalPatrolPoints = SplineComp->GetNumberOfSplinePoints(); // Get the total number of patrol points
	
	// Loop back to the first patrol point if we've reached the end
	if (CurrentIndex >= TotalPatrolPoints)
	{
		CurrentIndex = 0;
	}

	BlackboardComp->SetValueAsInt("PatrolIndex", CurrentIndex); // Update the patrol index in the blackboard

	return EBTNodeResult::Succeeded;
}