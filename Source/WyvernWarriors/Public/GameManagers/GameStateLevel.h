#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GameStateLevel.generated.h"

UCLASS()
class WYVERNWARRIORS_API AGameStateLevel : public AGameStateBase
{
	GENERATED_BODY()

public:
	// Returns the current wave number
	UFUNCTION(BlueprintCallable, Category = "Wave")
	int32 GetCurrentWaveNumber() const { return CurrentWaveNumber; } 

	// Sets the current wave number
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void SetCurrentWaveNumber(int32 const NewWaveNumber) { if (HasAuthority()) CurrentWaveNumber = NewWaveNumber;  } 

	// Returns the current value of the control meter
	UFUNCTION(BlueprintCallable, Category = "Control Meter")
	float GetControlMeterCurrent() const { return ControlMeterCurrent; } 

	// Sets the current value of the control meter
	UFUNCTION(BlueprintCallable, Category = "Control Meter")
	void SetControlMeterCurrent(float const NewControlNumber) { if (HasAuthority()) ControlMeterCurrent = NewControlNumber; }; 

private:
	// Current wave number
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wave", meta = (AllowPrivateAccess = true))
	int32 CurrentWaveNumber = 1; 

	// Current value of the control meter
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control Meter", meta = (AllowPrivateAccess = true))
	float ControlMeterCurrent = 0; 
};
