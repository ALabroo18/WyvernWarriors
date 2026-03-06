// Fill out your copyright notice in the Description page of Project Settings.


#include "GruntEnemyController.h"
#include "GruntEnemy.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISenseConfig_Sight.h"

class UBlackboardData;

// Set default values for this controller's properties
AGruntEnemyController::AGruntEnemyController()
{
	GruntBlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("GruntBlackboardComponent")); // Initialize the blackboard component
	GruntAIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("GruntAIPerceptionComponent")); // Initialize the AI perception component
	
	UAISenseConfig_Sight* SightConfig =  CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("GruntSightConfig")); // Initialize the sight config for perception
	GruntAIPerceptionComponent->ConfigureSense(*SightConfig); // Configure sight on AI perception
	GruntAIPerceptionComponent->SetDominantSense(UAISenseConfig_Sight::StaticClass()); // Set sight as primary sense
}

// Sets up behavior tree and blackboard then runs tree when possessing grunt
void AGruntEnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn); // Call base class possess logic

	// Cast the possessed pawn to AGruntEnemy
	const AGruntEnemy* ControlledGruntEnemy = Cast<AGruntEnemy>(GetPawn());
	if(!IsValid(ControlledGruntEnemy))
	{
		return;
	}

	// Initialize the blackboard with the behavior tree's asset
	GruntBlackboardComponent->InitializeBlackboard(*GruntBehaviorTree->BlackboardAsset);
	if (!IsValid(GruntBlackboardComponent))
	{
		return;
	}

	GruntBlackboardComponent->SetValueAsBool(FName(TEXT("bOnPatrolRoute")), ControlledGruntEnemy->GetOnPatrolRoute()); // Set the controlled grunt enemy in the blackboard
	GruntBlackboardComponent->SetValueAsObject(FName(TEXT("PlayerWyvern")), UGameplayStatics::GetPlayerPawn(GetWorld(), 0)); // Set the controlled grunt enemy in the blackboard

	RunBehaviorTree(GruntBehaviorTree); // Start the behavior tree for AI logic
}