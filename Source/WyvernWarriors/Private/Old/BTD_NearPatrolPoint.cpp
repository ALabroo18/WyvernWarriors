// Fill out your copyright notice in the Description page of Project Settings.


#include "Old/BTD_NearPatrolPoint.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SplineComponent.h"

bool UBTD_NearPatrolPoint::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	// Get the AI controller
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		// Get the blackboard component
		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
		if (BlackboardComp)
		{
			// Get the patrol route
			AActor* PatrolRoute = Cast<AActor>(BlackboardComp->GetValueAsObject("PatrolRoute"));
			if (PatrolRoute) {

				// Get the spline component from the patrol route actor
				USplineComponent* SplineComp = PatrolRoute->FindComponentByClass<USplineComponent>();
				if (SplineComp) {

					// Get the location of the current patrol point and the AI pawn
					FVector PatrolPointLocation = SplineComp->GetLocationAtSplinePoint(BlackboardComp->GetValueAsInt("PatrolIndex"), ESplineCoordinateSpace::World);
					APawn* AIPawn = AIController->GetPawn();
					if (AIPawn)
					{
						// Calculate distance and check if within threshold
						float Distance = FVector::Dist(PatrolPointLocation, AIPawn->GetActorLocation());
						return Distance <= NearDistance;
					}
				}
			}
		}
	}

	return false;
}