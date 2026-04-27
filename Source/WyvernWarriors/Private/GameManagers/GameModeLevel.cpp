#include "GameManagers/GameModeLevel.h"
#include "BossEnemy.h"
#include "GameManagers/GameStateLevel.h"
#include "GameManagers/Components/CannonManagerComponent.h"
#include "GameManagers/Components/EnemyManagerComponent.h"
#include "GameManagers/Components/WaveManagerComponent.h"
#include "GameManagers/Components/EventBusComponent.h"

// Sets default values for this actor's properties
AGameModeLevel::AGameModeLevel()
{
	GameStateClass = AGameStateLevel::StaticClass(); // Set the game state class to A_GameStateLevel

	WaveManager = CreateDefaultSubobject<UWaveManagerComponent>("WaveManager"); // Create wave manager
	EnemyManager = CreateDefaultSubobject<UEnemyManagerComponent>("EnemyManager"); // Create enemy manager
	CannonManager = CreateDefaultSubobject<UCannonManagerComponent>("CannonManager"); // Create cannon manager
	EventBus = CreateDefaultSubobject<UEventBusComponent>("EventBus"); // Create event bus
}

/* Get reference to game state and set up game manager components. End by starting first wave.
 */
void AGameModeLevel::BeginPlay()
{
	Super::BeginPlay();
	
	GameStateLevel = Cast<AGameStateLevel>(GetWorld()->GetGameState());
	if (!IsValid(GameStateLevel))
	{
		return;
	}

	CannonManager->SetupCannonManager();
	EnemyManager->SetupEnemyManager();
	WaveManager->SetupWaveManager();
	
	EventBus->OnBossStateChange.AddDynamic(this, &AGameModeLevel::OnBossDefeated);
	GetWorldTimerManager().SetTimer(
		LevelWinHandle,
		this,
		&AGameModeLevel::StartGane,
		1,
		false
		);
}

/* Start the first wave.
 */
void AGameModeLevel::StartGane()
{
	WaveManager->NewWave();
}

/* 
 */
void AGameModeLevel::OnBossDefeated(EBossState const NewState)
{
	if (NewState == EBossState::Defeated)
	{
		UE_LOG(LogTemp, Log, TEXT("Boss has been defeated, set timer to win level."))
		GetWorldTimerManager().SetTimer(
			LevelWinHandle,
			this,
			&AGameModeLevel::WinLevel,
			LevelWinHandleTime,
			false
		);
	}
}