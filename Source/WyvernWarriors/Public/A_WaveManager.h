// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "A_WaveManager.generated.h"

UCLASS()
class WYVERNWARRIORS_API AA_WaveManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AA_WaveManager();
	virtual void Tick(float DeltaTime) override;
	UFUNCTION()
	void WaveCompleted();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	int32 CurrentWave;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	float ControlMeterMax;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	float ControlMeterCurrent;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	AActor* EnemySpawner;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	TArray<AActor*> Outpost;

};
