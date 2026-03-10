#include "GameManagers/Components/WaveManagerComponent.h"
#include "GameManagers/GameModeLevel.h"
#include "GameManagers/GameStateLevel.h"
#include "GameManagers/Components/EventBusComponent.h"
#include "GameManagers/Components/EnemyManagerComponent.h"
#include "Outpost.h"
#include "GameManagers/Components/CannonManagerComponent.h"
#include "Kismet/GameplayStatics.h"

// Completes the current wave and prepares for the next one
void UWaveManagerComponent::WaveCompleted()
{
	// If completing final wave, win the game
	if (bIsFinalWave)
	{
		GameModeLevel->WinLevel();
		return;
	}
	
	// Increment current wave number
	int32 CurrentWaveNumber = GameStateLevel->GetCurrentWaveNumber();
	CurrentWaveNumber++;

	GameStateLevel->SetCurrentWaveNumber(CurrentWaveNumber); // Update wave number in game state
	
	EventBus->OnWaveComplete.Broadcast(bIsFinalWave); // Trigger wave completed delegate
	NewWave(); // Start new wave
}

// Starts a new wave
void UWaveManagerComponent::NewWave()
{
	// Check if final wave reached and set flag accordingly
	if (int32 const CurrentWaveNumber = GameStateLevel->GetCurrentWaveNumber(); CurrentWaveNumber == FinalWaveNumber)
	{
		bIsFinalWave = true;

		UEnemyManagerComponent* EnemyManager = GameModeLevel->GetEnemyManagementComponent();
		UCannonManagerComponent* CannonManager = GameModeLevel->GetCannonManager();
		EventBus->OnNewWave.AddDynamic(EnemyManager, &UEnemyManagerComponent::OnNewWave);
		EventBus->OnNewWave.AddDynamic(CannonManager, &UCannonManagerComponent::OnNewWave);
	}
	else
	{
		RecaptureOutposts(); // Recaptures outposts
		ResetControlMeter(); // Reset control meter
	}

	EventBus->OnNewWave.Broadcast(bIsFinalWave); // Trigger new wave delegate
}

// Recaptures outposts based on outpost and wave status
void UWaveManagerComponent::RecaptureOutposts()
{
	// Early out if no outposts
	if (Outposts.IsEmpty())
	{
		return;
	}

	TArray<AOutpost*> PlayerControlledOutposts; // Array to store player-controlled outposts
	
	// Iterate through all outposts; capture all for final wave, otherwise store player-controlled ones
	for (AOutpost* Outpost : Outposts)
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
	
	int32 const TotalOutposts = Outposts.Num();

	// If player controls less than half of outposts, recapture all
	if (int32 const NumPlayerOutposts = PlayerControlledOutposts.Num(); NumPlayerOutposts <= TotalOutposts / 2)
	{
		for (AOutpost* Outpost : PlayerControlledOutposts)
		{
			if (IsValid(Outpost))
			{
				Outpost->NewWave(bIsFinalWave); // Recapture player-controlled outposts
			}
		}
	}

	// Calculate number of outposts to recapture based on even/odd total outposts
	int32 NumOutpostsToRecapture;
	if (TotalOutposts % 2 == 0)
	{
		NumOutpostsToRecapture = TotalOutposts / 2; // Even number of outposts: half
	}
	else
	{
		NumOutpostsToRecapture = TotalOutposts / 2 + 1; // Odd number of outposts: half rounded up
	}
	
	// Randomly select outposts to recapture
	for (int32 i = 0; i < NumOutpostsToRecapture; i++)
	{
		int32 const RandomIndex = FMath::RandRange(0, PlayerControlledOutposts.Num() - 1); // Get random index
		AOutpost* OutpostToRecapture = PlayerControlledOutposts[RandomIndex]; // Select outpost to recapture

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
		FTimerDelegate::CreateUObject(this, &UWaveManagerComponent::ModifyControlMeterCurrent, ResetDecreaseAmount, true), // Delegate with decrease amount and bypass flag
		ResetDecreaseInterval,
		true);
}

// Sets the starting variables for wave management
void UWaveManagerComponent::SetStartVariables()
{
	GameModeLevel = Cast<AGameModeLevel>(GetOwner()); // Get reference to game mode level
	GameStateLevel = Cast<AGameStateLevel>(GameModeLevel->GetGameStateLevelRef()); // Get reference to game state level
	EventBus = Cast<UEventBusComponent>(GameModeLevel->GetEventBusComponent()); // Get event bus

	GameStateLevel->SetCurrentWaveNumber(StartWaveNumber); // Set current wave number to starting wave number
	GameStateLevel->SetControlMeterCurrent(ControlMeterStart); // Set control meter to starting value
	bIsFinalWave = false; // Reset final wave flag

	// Get all outposts
	TArray<AActor*> TempOutposts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AOutpost::StaticClass(), TempOutposts);

	// Set all outpost as player captured and add to outpost array
	for (AActor* TempOutpost : TempOutposts)
	{
		if (IsValid(TempOutpost))
		{
			AOutpost* Outpost = Cast<AOutpost>(TempOutpost);
			Outpost->SetPlayerCapturedStatus(true);
			Outposts.Add(Cast<AOutpost>(Outpost));
		}
	}
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
	
	float ControlMeterCurrent = GameStateLevel->GetControlMeterCurrent(); // Get current control meter value

	if (ControlMeterCurrent >= ControlMeterMax && !bResettingControlMeter)
	{
		bResettingControlMeter = true; // Start resetting control meter
		WaveCompleted(); // Complete wave if control meter maxed
		return;
	}
	
	// Lose level if control meter is 0 or below
	if (ControlMeterCurrent <= 0)
	{
		GameModeLevel->LoseLevel();
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
