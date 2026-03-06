// Fill out your copyright notice in the Description page of Project Settings.


#include "Old/BTT_AvoidOtherActors.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Components/SphereComponent.h"

EBTNodeResult::Type UBTT_AvoidOtherActors::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	// Get sphere component from controlled pawn
	USphereComponent* SphereComp = ControlledPawn->FindComponentByClass<USphereComponent>();
	if (!SphereComp)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	// Get overlapping actors of the enemy class
	TArray<AActor*> OverlappingActors;
	SphereComp->GetOverlappingActors(OverlappingActors);

	// Remove the controlled pawn from overlapping actors
	if (OverlappingActors.Contains(ControlledPawn))
	{
		OverlappingActors.Remove(ControlledPawn);
	}

	if (OverlappingActors.IsEmpty())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return EBTNodeResult::Succeeded;
	}

	// Get blackboard component
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
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
	
	// Remove specified classes from overlapping actors
	OverlappingActors.RemoveAll([&](AActor* Actor)
	{
		for (auto ClassType : ClassesToRemove)
		{
			if (Actor->IsA(ClassType))
			{
				return false;
			}
		}
		return false;
	});

	// If no overlapping actors remain after removal, finish task
	if (OverlappingActors.IsEmpty())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return EBTNodeResult::Succeeded;
	}

	// Get an array of directions away from each overlapping actor
	TArray<FVector> DirectionAwayFromEnemies;
	for (AActor* Actor : OverlappingActors)
	{
		FVector DirectionAway = (ControlledPawn->GetActorLocation() - Actor->GetActorLocation()).GetSafeNormal();
		DirectionAwayFromEnemies.Add(DirectionAway);
	}

	// Calculate the average direction away from all overlapping actors
	FVector DirectionAwaySum = FVector::ZeroVector;
	for (const FVector& Vec : DirectionAwayFromEnemies)
	{
		DirectionAwaySum += Vec;
	}
	FVector DirectionAwayAverage = FVector::ZeroVector;
	DirectionAwayAverage = DirectionAwaySum / DirectionAwayFromEnemies.Num();

	// Move focus point away from overlapping actors
	FVector currentLocation = FocusPoint->GetActorLocation(); // Current location of the focus point
	FVector desiredLocation = ControlledPawn->GetActorLocation() + DirectionAwayAverage * BlackboardComp->GetValueAsFloat("Speed"); // Desired location to avoid other actors
	FVector newLocation = FMath::VInterpTo(currentLocation, desiredLocation, OwnerComp.GetWorld()->GetDeltaSeconds(), 5.0f); // Interpolated new location based on average direction away from enemies
	FocusPoint->SetActorLocation(newLocation); // Set the new location of the focus

	
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
}
