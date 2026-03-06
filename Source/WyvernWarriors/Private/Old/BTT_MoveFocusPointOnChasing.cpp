// Fill out your copyright notice in the Description page of Project Settings.


#include "Old/BTT_MoveFocusPointOnChasing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Components/SphereComponent.h"

EBTNodeResult::Type UBTT_MoveFocusPointOnChasing::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Get the blackboard component
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	// Get the AI controller and check if it's valid
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	// Get the controlled pawn
	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
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

	// Get player actor from blackboard
	AActor* Player = Cast<AActor>(BlackboardComp->GetValueAsObject("Player"));
	if (!Player)
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
	SphereComp->GetOverlappingActors(OverlappingActors, EnemyClass);

	// Remove the controlled pawn from overlapping actors
	if (OverlappingActors.Contains(ControlledPawn))
	{
		OverlappingActors.Remove(ControlledPawn);
	}

	FVector currentLocation = FocusPoint->GetActorLocation(); // Current location of the focus point
	FVector directionToMove = FVector::ZeroVector; // Direction to move the focus point
	FVector directionAwayAverage = FVector::ZeroVector; // Average direction away from overlapping actors

	if (!OverlappingActors.IsEmpty())
	{
		// Get an array of directions away from each overlapping actor
		TArray<FVector> directionAwayFromEnemies;
		for (AActor* Actor : OverlappingActors)
		{
			FVector directionAway = (ControlledPawn->GetActorLocation() - Actor->GetActorLocation()).GetSafeNormal();
			directionAwayFromEnemies.Add(directionAway);
		}

		// Calculate the average direction away from all overlapping actors
		FVector directionAwaySum = FVector::ZeroVector;
		for (const FVector& Vec : directionAwayFromEnemies)
		{
			directionAwaySum += Vec;
		}
		directionAwayAverage = directionAwaySum / directionAwayFromEnemies.Num();
	}
	
	// Determine movement mode and calculate direction to move
	switch (moveMode)
	{
		case EMoveFocusPointMode::ChasePlayer:
		{
			FVector directionToPlayer = (Player->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal(); // Get direction to player
			directionToMove = (directionToPlayer + directionAwayAverage).GetSafeNormal(); // Combine direction to player and away from enemies
			break;
		}
		case EMoveFocusPointMode::CirclePlayer:
		{
			FVector directionToPlayer = (Player->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal(); // Get direction to player
			FVector directionToMoveRight = FVector::CrossProduct(directionToMove, FVector::UpVector).GetSafeNormal(); // Get right vector for circling
			directionToMove = (directionToMoveRight + directionAwayAverage).GetSafeNormal(); // Combine circling direction and away from enemies
			break;
		}
		case EMoveFocusPointMode::FleePlayer:
		{
			FVector directionAwayFromPlayer = (ControlledPawn->GetActorLocation() - Player->GetActorLocation()).GetSafeNormal(); // Get direction away from player
			directionToMove = (directionAwayFromPlayer + directionAwayAverage).GetSafeNormal(); // Combine direction away from player and away from enemies
			break;
		}
	}

	FVector desiredLocation = ControlledPawn->GetActorLocation() + directionToMove * BlackboardComp->GetValueAsFloat("Speed"); // Desired location to move the focus point
	FVector newLocation = FMath::VInterpTo(currentLocation, desiredLocation, OwnerComp.GetWorld()->GetDeltaSeconds(), 5.0f); // Interpolated new location based on desired location
	FocusPoint->SetActorLocation(newLocation); // Set the new location of the focus

	FVector NewFocusPointLocation = FocusPoint->GetActorLocation(); // Get the updated location of the focus point
	
	// Ensure the focus point does not go below 0 on the Z-axis
	if (NewFocusPointLocation.Z < 0.f)
	{
		FocusPoint->SetActorLocation(FVector(NewFocusPointLocation.X, NewFocusPointLocation.Y, 0.f));
	}

	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
}