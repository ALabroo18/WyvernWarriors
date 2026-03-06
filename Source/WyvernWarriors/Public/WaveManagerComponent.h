#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WaveManagerComponent.generated.h"

class AOutpost;
class ACannon;
class ABossEnemy;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WYVERNWARRIORS_API UWaveManagerComponent : public UActorComponent
{
	GENERATED_BODY()
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewWave, bool, BIsFinalWave); // Delegate for new wave event
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveComplete, bool, BIsFinalWave); // Delegate for wave completion event

public:
	// Event triggered when a new wave happens
	UPROPERTY(BlueprintAssignable, Category = "Wave")
	FOnNewWave OnNewWave; 

	// Event triggered when a wave is completed
	UPROPERTY(BlueprintAssignable, Category = "Wave")
	FOnWaveComplete OnWaveComplete;
	
	// Sets the starting variables for wave management
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void SetStartVariables(); 
	
	// Completes the current wave
	UFUNCTION(Category = "Wave")
	void WaveCompleted() const; 
	
	// Starts a new wave
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void NewWave();
	
	// Recaptures outposts
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void RecaptureOutposts(); 
	
	// Resets the control meter to half its maximum value over time
	UFUNCTION(BlueprintCallable, Category = "Control Meter")
	void ResetControlMeter();
	
	// Modifies the current value of the control meter
	UFUNCTION(BlueprintCallable, Category = "Control Meter")
	void ModifyControlMeterCurrent(float Amount, bool bCanBypassReset); 

private:
	// Timer handle for control meter reset
	FTimerHandle ControlMeterResetTimerHandle;
	
	// Starting wave number
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave", meta = (AllowPrivateAccess = true))
	int32 StartWaveNumber = 1; 
	
	// Final wave number
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave", meta = (AllowPrivateAccess = true))
	int32 FinalWaveNumber = 3; 
	
	// Whether the final wave has been reached
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wave", meta = (AllowPrivateAccess = true))
	bool bIsFinalWave = false; 
	
	// Array of outposts in the level
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wave", meta = (AllowPrivateAccess = true))
	TArray<TObjectPtr<AOutpost>> Outposts;
	
	// Amount to decrease control meter by timer interval
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Control Meter", meta = (AllowPrivateAccess = true))
	float ControlMeterDecreaseAmount = -1.f; 
	
	// Time interval for control meter decrease
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Control Meter", meta = (AllowPrivateAccess = true))
	float ControlMeterDecreaseInterval = 0.1f; 
	
	// Maximum value of the control meter
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Control Meter", meta = (AllowPrivateAccess = true))
	float ControlMeterMax = 1500.f; 
	
	// Starting value of the control meter on new wave
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Control Meter", meta = (AllowPrivateAccess = true))
	float ControlMeterStart = 750.f;
	
	// Whether the control meter is currently resetting
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control Meter", meta = (AllowPrivateAccess = true))
	bool bResettingControlMeter = false; 
};
