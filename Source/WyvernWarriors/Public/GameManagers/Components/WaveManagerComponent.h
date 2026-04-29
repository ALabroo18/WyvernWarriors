#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WaveManagerComponent.generated.h"

class AGameStateLevel;
class AGameModeLevel;
class UEventBusComponent;
class AOutpost;
class ACannon;
class ABossEnemy;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WYVERNWARRIORS_API UWaveManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets the starting variables for wave management
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void SetupWaveManager(); 
	
	// Completes the current wave
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void WaveCompleted(); 
	
	// Starts a new wave
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void NewWave();
	
	// Recaptures outposts
	UFUNCTION(BlueprintCallable, Category = "Outpost")
	void RecaptureOutposts(); 
	
	// Returns a random captured outpost
	UFUNCTION(BlueprintCallable, Category = "Outpost")
	AOutpost* GetRandomCapturedOutpost();
	
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Outpost", meta = (AllowPrivateAccess = true))
	TArray<AOutpost*> Outposts;
	
	// Amount to decrease control meter when resetting
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Control Meter", meta = (AllowPrivateAccess = true))
	float ResetDecreaseAmount = -25.f; 
	
	// Time interval for control meter decrease when resetting
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Control Meter", meta = (AllowPrivateAccess = true))
	float ResetDecreaseInterval = 0.1f; 
	
	// Maximum value of the control meter
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Control Meter", meta = (AllowPrivateAccess = true))
	float ControlMeterMax = 1000.f; 
	
	// Starting value of the control meter on new wave
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Control Meter", meta = (AllowPrivateAccess = true))
	float ControlMeterStart = 500.f;
	
	// Whether the control meter is currently resetting
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control Meter", meta = (AllowPrivateAccess = true))
	bool bResettingControlMeter = false; 
	
	// Event bus for delegates
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Manager", meta = (AllowPrivateAccess = true))
	UEventBusComponent* EventBus;

	// Reference to game mode level
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Manager", meta = (AllowPrivateAccess = true))
	AGameModeLevel* GameModeLevel;

	// Reference to game state level
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Manager", meta = (AllowPrivateAccess = true))
	AGameStateLevel* GameStateLevel;
};
