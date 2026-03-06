// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_MoveFocusPointOnChasing.generated.h"

UENUM(BlueprintType)
enum class EMoveFocusPointMode : uint8
{
	ChasePlayer UMETA(DisplayName = "Chase Player"),
	FleePlayer UMETA(DisplayName = "Flee Player"),
	CirclePlayer UMETA(DisplayName = "Circle Player"),
};

UCLASS()
class WYVERNWARRIORS_API UBTT_MoveFocusPointOnChasing : public UBTTaskNode
{
	GENERATED_BODY()

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	EMoveFocusPointMode moveMode;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	TSubclassOf<AActor> EnemyClass;
};
