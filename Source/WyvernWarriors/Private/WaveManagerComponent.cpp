#include "WaveManagerComponent.h"
#include "GameModeLevel.h"
#include "GameStateLevel.h"
#include "Outpost.h"

// Completes the current wave and prepares for the next one
void UWaveManagerComponent::WaveCompleted() const
{
	// If completing final wave, win the game
	if (bIsFinalWave)
	{
		Cast<AGameModeLevel>(GetOwner())->WinLevel();
		return;
	}
	
	// Increment current wave number
	int32 CurrentWaveNumber = Cast<AGameModeLevel>(GetOwner())->GetGameStateLevelRef()->GetCurrentWaveNumber();
	CurrentWaveNumber++;

	Cast<AGameModeLevel>(GetOwner())->GetGameStateLevelRef()->SetCurrentWaveNumber(CurrentWaveNumber); // Update wave number in game state
	
	OnWaveComplete.Broadcast(bIsFinalWave);
}

// Starts a new wave
void UWaveManagerComponent::NewWave()
{
	int32 CurrentWaveNumber = Cast<AGameModeLevel>(GetOwner())->GetGameStateLevelRef()->GetCurrentWaveNumber();
	
	// Check if final wave reached; set flag accordingly
	if (CurrentWaveNumber == FinalWaveNumber)
	{
		bIsFinalWave = true;
	}

	OnNewWave.Broadcast(bIsFinalWave); // Trigger wave completed event
}

// Recaptures outposts based on outpost and wave status
void UWaveManagerComponent::RecaptureOutposts()
{
	// Early out if no outposts
	if (Outposts.IsEmpty())
	{
		return;
	}

	// Capture all outposts for final wave
	if (bIsFinalWave)
	{
		for (TObjectPtr<AOutpost> Outpost : Outposts)
		{
			if (Outpost)
			{
				Outpost->NewWave(bIsFinalWave);
			}
		}
		return;
	}

	TArray<TObjectPtr<AOutpost>> PlayerControlledOutposts; // Array to store player-controlled outposts
	
	// Iterate through all outposts; capture all for final wave, otherwise store player-controlled ones
	for (TObjectPtr<AOutpost> Outpost : Outposts)
	{
		if (IsValid(Outpost))
		{
			if (Outpost->GetPlayerCapturedStatus())
			{
				PlayerControlledOutposts.Add(Outpost); // Store player-controlled outposts
			}
		}
	}

	// Early out if no player-controlled outposts
	if (PlayerControlledOutposts.IsEmpty())
	{
		return;
	}
	
	int32 NumPlayerOutposts = PlayerControlledOutposts.Num(); // Number of player-controlled outposts

	// If player controls less than half of outposts, recapture all
	if (NumPlayerOutposts <= Outposts.Num() / 2)
	{
		for (TObjectPtr<AOutpost> Outpost : PlayerControlledOutposts)
		{
			if (IsValid(Outpost))
			{
				Outpost->NewWave(bIsFinalWave); // Recapture player-controlled outposts
			}
		}
	}

	// Calculate number of outposts to recapture based on even/odd total outposts
	int32 NumOutpostsToRecapture;
	int32 TotalOutposts = Outposts.Num();
	if (TotalOutposts % 2 == 0)
	{
		NumOutpostsToRecapture = TotalOutposts / 2; // Even number of outposts: half minus one
	}
	else
	{
		NumOutpostsToRecapture = TotalOutposts / 2 + 1; // Odd number of outposts: half rounded down
	}
	
	// Randomly select outposts to recapture
	for (int32 i = 0; i < NumOutpostsToRecapture; i++)
	{
		int32 const RandomIndex = FMath::RandRange(0, PlayerControlledOutposts.Num() - 1); // Get random index
		TObjectPtr<AOutpost> OutpostToRecapture = PlayerControlledOutposts[RandomIndex]; // Select outpost to recapture

		if (IsValid(OutpostToRecapture))
		{
			OutpostToRecapture->NewWave(bIsFinalWave); // Recapture selected outpost
			PlayerControlledOutposts.RemoveAt(RandomIndex); // Remove from list to avoid duplicate recaptures
		}
	}
}

// Resets the control meter to half its maximum value over time
void UWaveManagerComponent::ResetControlMeter()
{
	// Start timer to decrease control meter over an interval
	GetWorld()->GetTimerManager().SetTimer( 
		ControlMeterResetTimerHandle,
		FTimerDelegate::CreateUObject(this, &UWaveManagerComponent::ModifyControlMeterCurrent, ControlMeterDecreaseAmount, true), // Delegate with decrease amount and bypass flag
		ControlMeterDecreaseInterval,
		true);
}

// Sets the starting variables for wave management
void UWaveManagerComponent::SetStartVariables()
{
	Cast<AGameModeLevel>(GetOwner())->GetGameStateLevelRef()->SetCurrentWaveNumber(StartWaveNumber); // Set current wave number to starting wave number
	Cast<AGameModeLevel>(GetOwner())->GetGameStateLevelRef()->SetControlMeterCurrent(ControlMeterStart); // Set control meter to starting value
	bIsFinalWave = false; // Reset final wave flag
}

// Modifies the current value of the control meter
void UWaveManagerComponent::ModifyControlMeterCurrent(float Amount, bool bCanBypassReset)
{
	// Do not modify control meter if it is resetting
	if (bResettingControlMeter)
	{
		if (bCanBypassReset == false)
		{
			return;
		}
	}

	// Early out if final wave reached
	if (bIsFinalWave)
	{
		return;
	}

	AGameStateLevel* GameStateLevel = Cast<AGameModeLevel>(GetOwner())->GetGameStateLevelRef();
	
	float ControlMeterCurrent = GameStateLevel->GetControlMeterCurrent(); // Get current control meter value

	if (ControlMeterCurrent >= ControlMeterMax && !bResettingControlMeter)
	{
		bResettingControlMeter = true; // Start resetting control meter
		WaveCompleted(); // Complete wave if control meter maxed
		return;
	}

	ControlMeterCurrent += Amount; // Modify control meter by amount

	ControlMeterCurrent = FMath::Clamp(ControlMeterCurrent, 0.0f, ControlMeterMax); // Clamp control meter value

	GameStateLevel->SetControlMeterCurrent(ControlMeterCurrent); // Update control meter in game state

	if (bCanBypassReset && ControlMeterCurrent <= ControlMeterStart)
	{
		GameStateLevel->SetControlMeterCurrent(ControlMeterStart); // Set to wave start value
		bResettingControlMeter = false; // Stop resetting if bypassed
		GetWorld()->GetTimerManager().ClearTimer(ControlMeterResetTimerHandle); // Clear timer
	}
}
