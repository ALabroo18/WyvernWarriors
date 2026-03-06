#include "GameModeLevel.h"

#include "CannonManagerComponent.h"
#include "EnemyManagerComponent.h"
#include "GameStateLevel.h"
#include "WaveManagerComponent.h"

// Sets default values for this actor's properties
AGameModeLevel::AGameModeLevel()
{
	GameStateClass = AGameStateLevel::StaticClass(); // Set the game state class to A_GameStateLevel
	
	WaveManagementComponent = CreateDefaultSubobject<UWaveManagerComponent>("WaveManager");
	EnemyManagementComponent = CreateDefaultSubobject<UEnemyManagerComponent>("EnemyManager");
	CannonManagementComponent = CreateDefaultSubobject<UCannonManagerComponent>("CannonManager");
}