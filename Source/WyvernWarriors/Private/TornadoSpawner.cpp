#include "TornadoSpawner.h"
#include "Tornado.h"
#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

ATornadoSpawner::ATornadoSpawner()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATornadoSpawner::BeginPlay()
{
    Super::BeginPlay();
}

void ATornadoSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (IsValid(ActiveTornado)) return;

    CooldownAccumulator += DeltaTime;
    if (CooldownAccumulator < SpawnCooldown) return;

    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Player) return;

    if (FVector::Dist(GetActorLocation(), Player->GetActorLocation()) <= SpawnProximityRadius)
    {
        TrySpawnTornado();
        CooldownAccumulator = 0.f;
    }
}

void ATornadoSpawner::TrySpawnTornado()
{
    if (!TornadoClass) return;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ATornado* NewTornado = GetWorld()->SpawnActor<ATornado>(
        TornadoClass, GetActorLocation(), FRotator::ZeroRotator, Params);

    if (!NewTornado) return;

    // Build the spline from our offset points then call UpdateSpline
    // so GetSplineLength() returns a valid value for movement
    USplineComponent* Spline = NewTornado->SplinePath;
    Spline->ClearSplinePoints();

    for (int32 i = 0; i < SplinePointOffsets.Num(); i++)
        Spline->AddSplinePoint(GetActorLocation() + SplinePointOffsets[i], ESplineCoordinateSpace::World);

    Spline->UpdateSpline();
    ActiveTornado = NewTornado;
}