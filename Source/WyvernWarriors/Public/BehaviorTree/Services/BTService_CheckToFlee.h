#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckToFlee.generated.h"

class AGruntEnemy;

UCLASS()
class WYVERNWARRIORS_API UBTService_CheckToFlee : public UBTService
{
	GENERATED_BODY()
	
protected:
	// Sets default values for service
	UBTService_CheckToFlee();
	
	// Get blackboard values when service is running
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	// Tick to check if enemy should flee from player
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	// Distance to player at which enemy should switch to fleeing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	float FleeingDistance = 30000.f;
	
	// Blackboard key for bool for fleeing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector FleeingKey;
	
	// Blackboard key for the player wyvern
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector DistanceToPlayerKey;
	
	// Blackboard key for the controlled grunt.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector SelfGruntKey;
	
	// Reference to grunt enemy self
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	AGruntEnemy* GruntEnemy;
};
