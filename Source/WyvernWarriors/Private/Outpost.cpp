#include "Outpost.h"

/* Sets maximum amount of grunts allowed on outpost patrol route
 */ 
void AOutpost::BeginPlay()
{
	Super::BeginPlay();
	
	OutpostPatrolRoute->SetMaxEnemiesOnRoute(MaxGruntOnRoute);
}
