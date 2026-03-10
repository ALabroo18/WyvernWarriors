#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckIsAggressive.generated.h"

class AGruntEnemy;

UCLASS()
class WYVERNWARRIORS_API UBTService_CheckIsAggressive : public UBTService
{
	GENERATED_BODY()
	
protected:
	// Sets default values for service
	UBTService_CheckIsAggressive();
	
	// Get blackboard values when service is running
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	// Tick to check if enemy should flee from player
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	// Distance to player at which enemy should switch to fleeing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	float AggressionDistance = 50000.f;
	
	// Blackboard key for bool for fleeing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector AggressionKey;
	
	// Blackboard key for distance to the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector DistanceToPlayerKey;
	
	// Reference to the player
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	AActor* PlayerCharacter;
};
