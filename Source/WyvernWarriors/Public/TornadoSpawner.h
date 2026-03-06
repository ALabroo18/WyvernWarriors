#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TornadoSpawner.generated.h"

class ATornado;

UCLASS()
class WYVERNWARRIORS_API ATornadoSpawner : public AActor
{
    GENERATED_BODY()

public:
    ATornadoSpawner();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Assign BP_Tornado here in the editor
    UPROPERTY(EditAnywhere, Category = "Spawner")
    TSubclassOf<ATornado> TornadoClass;

    UPROPERTY(EditAnywhere, Category = "Spawner")
    float SpawnProximityRadius = 5000.f;

    UPROPERTY(EditAnywhere, Category = "Spawner")
    float SpawnCooldown = 30.f;

    // Path points relative to this spawner. Needs at least 2 entries.
    UPROPERTY(EditAnywhere, Category = "Spawner")
    TArray<FVector> SplinePointOffsets;

private:
    float CooldownAccumulator = 0.f;

    // UPROPERTY keeps this from becoming a dangling pointer when
    // the tornado destroys itself. IsValid() will return false automatically.
    UPROPERTY()
    ATornado* ActiveTornado = nullptr;

    void TrySpawnTornado();
};