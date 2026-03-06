// Fill out your copyright notice in the Description page of Project Settings.


#include "Old/BTT_CirclePlayer.h"
#include "AiController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTT_CirclePlayer::UBTT_CirclePlayer()
{
	NodeName = "Circle Player";
}

EBTNodeResult::Type UBTT_CirclePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Get the AI Controller
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	// Get blackboard component
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
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

	FVector directionToPlayer = (Player->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal(); // Get direction to player
	FVector rightVector = FVector::CrossProduct(directionToPlayer, FVector::UpVector); // Get right vector for circling
	FVector desiredLocation = ControlledPawn->GetActorLocation() + rightVector * BlackboardComp->GetValueAsFloat("Speed"); // Desired location to circle around player
	FVector currentLocation = FocusPoint->GetActorLocation(); // Current location of the focus point
	FVector newLocation = FMath::VInterpTo(currentLocation, desiredLocation, OwnerComp.GetWorld()->GetDeltaSeconds(), 5.0f); // Interpolated new location
	FocusPoint->SetActorLocation(newLocation); // Set the new location of the focus
	
	
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
}