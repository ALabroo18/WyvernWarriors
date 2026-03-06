// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_CheckDistanceGreaterThan.generated.h"

/**
 *
 */
UCLASS()
class WYVERNWARRIORS_API UBTD_CheckDistanceGreaterThan : public UBTDecorator
{
	GENERATED_BODY()

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	UPROPERTY(EditAnywhere)
	float CheckDistance; // Distance that should be checked against
};