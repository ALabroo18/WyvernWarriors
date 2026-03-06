// Fill out your copyright notice in the Description page of Project Settings.

#include "Old/BTT_FocusPoint.h"
#include "AIController.h"
#include "GameFramework/Actor.h"
#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type UBTT_FocusPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)		
{
	// Get the AI controller
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (AIController)
    {
        UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
        if (BlackboardComp)
        {
            // Get the target actor from the blackboard
            AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(FocusTargetKey.SelectedKeyName));
            if (TargetActor)
            {
                // Set the focus on the target actor
                AIController->SetFocus(TargetActor);
                return EBTNodeResult::Succeeded;
            }
        }
    }
    
    return EBTNodeResult::Failed;
}