// Fill out your copyright notice in the Description page of Project Settings.


#include "Old/BTT_MoveFocusPointOnPatrolRoute.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SplineComponent.h"
#include "AIController.h"

EBTNodeResult::Type UBTT_MoveFocusPointOnPatrolRoute::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Get the AI Controller
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	// Get the controlled pawn
	APawn* ControlledPawn = AICon->GetPawn();
	if (!ControlledPawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	// Get the blackboard component
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	// Get the last point visited on the patrol route
	FVector PointOnPatrolRoute = BlackboardComp->GetValueAsVector("PointOnPatrolRoute");
	if (PointOnPatrolRoute.IsZero())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}
	
	// Get the focus point
	AActor* FocusPoint = Cast<AActor>(BlackboardComp->GetValueAsObject("FocusPoint"));
	if (!FocusPoint)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	if (FVector::Distance(ControlledPawn->GetActorLocation(), PointOnPatrolRoute) != 0)
	{
		FVector directionToPatrolRoute = (PointOnPatrolRoute - ControlledPawn->GetActorLocation()).GetSafeNormal(); // Get direction to point on patrol route
		FVector desiredLocation;
		if (FVector::Distance(ControlledPawn->GetActorLocation(), PointOnPatrolRoute) > BlackboardComp->GetValueAsFloat("Speed"))
		{
			desiredLocation = ControlledPawn->GetActorLocation() + directionToPatrolRoute * BlackboardComp->GetValueAsFloat("Speed"); // Desired location to move towards point on patrol route
		}
		else {
			desiredLocation = PointOnPatrolRoute; // Desired location to move towards point on patrol route
		}
		//FVector currentLocation = FocusPoint->GetActorLocation(); // Current location of the focus point
		// UE_LOG(LogTemp, Warning, TEXT("Current Location: %s"), *currentLocation.ToString());
		//FVector newLocation = FMath::VInterpTo(currentLocation, desiredLocation, OwnerComp.GetWorld()->GetDeltaSeconds(), 5.0f); // Interpolated new location based on desired location
		FocusPoint->SetActorLocation(desiredLocation); // Set the new location of the focus
	}

	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
}

