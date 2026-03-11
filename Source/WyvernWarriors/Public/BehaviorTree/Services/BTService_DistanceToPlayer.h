#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_DistanceToPlayer.generated.h"

class AGruntEnemy;

UCLASS()
class WYVERNWARRIORS_API UBTService_DistanceToPlayer : public UBTService
{
	GENERATED_BODY()
	
protected:
	// Sets default values for service
	UBTService_DistanceToPlayer();
	
	// Get blackboard values when service is running
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	// Tick to check if enemy should flee from player
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	// Blackboard key for the player wyvern
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector DistanceToPlayerKey;
	
	// Blackboard key for the player wyvern
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector SelfGruntKey;
	
	// Blackboard key for the player wyvern
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector PlayerCharacterKey;
	
	// Reference to the player
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	AActor* PlayerCharacter;
};
