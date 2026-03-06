// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_NearPatrolPoint.generated.h"

/**
 * 
 */
UCLASS()
class WYVERNWARRIORS_API UBTD_NearPatrolPoint : public UBTDecorator
{
	GENERATED_BODY()

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	UPROPERTY(EditAnywhere)
	float NearDistance = 200.0f; // Distance threshold to consider "near"
};
