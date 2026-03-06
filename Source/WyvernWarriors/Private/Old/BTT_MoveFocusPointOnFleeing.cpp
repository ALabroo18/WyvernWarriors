// Fill out your copyright notice in the Description page of Project Settings.


#include "Old/BTT_MoveFocusPointOnFleeing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

EBTNodeResult::Type UBTT_MoveFocusPointOnFleeing::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Get the AI controller and check if it's valid
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}
	
	// Get the AI pawn and check if it's valid
	APawn* AIPawn = AIController->GetPawn();
	if (!AIPawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	// Get the blackboard component and check if it's valid
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	// Get the focus point actor and check if it's valid
	AActor* FocusPoint = Cast<AActor>(BlackboardComp->GetValueAsObject("FocusPoint"));
	if (!FocusPoint)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	// Get the player actor and check if it's valid
	AActor* Player = Cast<AActor>(BlackboardComp->GetValueAsObject("Player"));
	if (!Player)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	FVector directionAwayFromPlayer = (AIPawn->GetActorLocation() - Player->GetActorLocation()).GetSafeNormal(); // Get direction away from player
	FVector desiredLocation = AIPawn->GetActorLocation() + directionAwayFromPlayer * BlackboardComp->GetValueAsFloat("Speed"); // Desired location to flee from the player
	FVector currentLocation = FocusPoint->GetActorLocation(); // Current location of the focus point
	FVector newLocation = FMath::VInterpTo(currentLocation, desiredLocation, OwnerComp.GetWorld()->GetDeltaSeconds(), 5.0f); // Interpolated new location based on average direction away from enemies
	FocusPoint->SetActorLocation(newLocation); // Set the new location of the focus

	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
}