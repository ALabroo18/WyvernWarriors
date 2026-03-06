// Fill out your copyright notice in the Description page of Project Settings.


#include "Old/BTD_CheckDistanceLessThan.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include <Kismet/GameplayStatics.h>
#include <GameFramework/Character.h>

bool UBTD_CheckDistanceLessThan::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{

	// Get the AI controller
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return false;
	}

	// Get blackboard component
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return false;
	}

	// Return whether the distance to the player is less than the check distance
	return BlackboardComp->GetValueAsFloat("DistanceToPlayer") < CheckDistance;
}
