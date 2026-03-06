// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTS_AvoidOtherActors.generated.h"

/**
 * 
 */
UCLASS()
class WYVERNWARRIORS_API UBTS_AvoidOtherActors : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTS_AvoidOtherActors();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	TArray<TSubclassOf<AActor>> ClassesToRemove;
};
