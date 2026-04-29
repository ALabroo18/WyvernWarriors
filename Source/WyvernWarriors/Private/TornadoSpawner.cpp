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

    if (IsValid(ActiveTornado)) { return; }

    CooldownAccumulator += DeltaTime;

    if (CooldownAccumulator < SpawnCooldown) return;

    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Player){ return; }

    const float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());

    if (Dist <= SpawnProximityRadius)
    {
        TrySpawnTornado();
        CooldownAccumulator = 0.f;
    }
}

void ATornadoSpawner::TrySpawnTornado()
{
    if (!TornadoClass) { return; }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ATornado* NewTornado = GetWorld()->SpawnActor<ATornado>(
        TornadoClass, GetActorLocation(), FRotator::ZeroRotator, Params);

    if (!NewTornado) { return; }

    USplineComponent* Spline = NewTornado->SplinePath;
    if (!Spline) { return; }

    if (SplinePointOffsets.Num() < 2) { return; }

    Spline->ClearSplinePoints();
    for (int32 i = 0; i < SplinePointOffsets.Num(); i++)
        Spline->AddSplinePoint(GetActorLocation() + SplinePointOffsets[i], ESplineCoordinateSpace::World);

    Spline->UpdateSpline();
    ActiveTornado = NewTornado;
}