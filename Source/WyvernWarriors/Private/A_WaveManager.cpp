// Fill out your copyright notice in the Description page of Project Settings.


#include "A_WaveManager.h"

// Sets default values
AA_WaveManager::AA_WaveManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AA_WaveManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AA_WaveManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ControlMeterCurrent >= ControlMeterMax)
	{
		WaveCompleted();
	}
}

void AA_WaveManager::WaveCompleted()
{
	CurrentWave++;

	
}

