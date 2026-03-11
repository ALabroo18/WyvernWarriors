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
    UE_LOG(LogTemp, Error, TEXT("TornadoSpawner: BeginPlay fired"));
}

void ATornadoSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (IsValid(ActiveTornado))
    {
        UE_LOG(LogTemp, Warning, TEXT("Spawner: Tornado already active, skipping"));
        return;
    }

    CooldownAccumulator += DeltaTime;
    UE_LOG(LogTemp, Warning, TEXT("Spawner: Cooldown %.1f / %.1f"), CooldownAccumulator, SpawnCooldown);

    if (CooldownAccumulator < SpawnCooldown) return;

    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Player)
    {
        UE_LOG(LogTemp, Warning, TEXT("Spawner: No player found"));
        return;
    }

    const float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    UE_LOG(LogTemp, Warning, TEXT("Spawner: Player distance %.1f / %.1f radius"), Dist, SpawnProximityRadius);

    if (Dist <= SpawnProximityRadius)
    {
        TrySpawnTornado();
        CooldownAccumulator = 0.f;
    }
}

void ATornadoSpawner::TrySpawnTornado()
{
    if (!TornadoClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Spawner: TornadoClass is null - assign BP_Tornado in Details"));
        return;
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ATornado* NewTornado = GetWorld()->SpawnActor<ATornado>(
        TornadoClass, GetActorLocation(), FRotator::ZeroRotator, Params);

    if (!NewTornado)
    {
        UE_LOG(LogTemp, Error, TEXT("Spawner: SpawnActor returned null"));
        return;
    }

    USplineComponent* Spline = NewTornado->SplinePath;
    if (!Spline)
    {
        UE_LOG(LogTemp, Error, TEXT("Spawner: SplinePath component is null on spawned tornado"));
        return;
    }

    if (SplinePointOffsets.Num() < 2)
    {
        UE_LOG(LogTemp, Error, TEXT("Spawner: SplinePointOffsets has %d points - needs at least 2"), SplinePointOffsets.Num());
        return;
    }

    Spline->ClearSplinePoints();
    for (int32 i = 0; i < SplinePointOffsets.Num(); i++)
        Spline->AddSplinePoint(GetActorLocation() + SplinePointOffsets[i], ESplineCoordinateSpace::World);

    Spline->UpdateSpline();
    ActiveTornado = NewTornado;

    UE_LOG(LogTemp, Warning, TEXT("Spawner: SUCCESS - tornado spawned with %d spline points, length %.1f"),
        SplinePointOffsets.Num(), Spline->GetSplineLength());
}