#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/CapsuleComponent.h"
#include "Tornado.generated.h"

UCLASS()
class WYVERNWARRIORS_API ATornado : public AActor
{
    GENERATED_BODY()

public:
    ATornado();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tornado")
    USplineComponent* SplinePath;

    // Overlap trigger — radius should match ForceOuterRadius (2x TornadoBodyRadius)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tornado")
    UCapsuleComponent* OuterZone;

    // Visual/reference shape for the funnel body — no collision
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tornado")
    UCapsuleComponent* InnerZone;

    UPROPERTY(EditAnywhere, Category = "Tornado|Movement")
    float MovementSpeed = 1200.f;

    // --- Damage ---

    // Damage per second at the tornado's eye (t = 0, closest)
    UPROPERTY(EditAnywhere, Category = "Tornado|Damage")
    float MaxDamagePerSecond = 15.f;

    // Damage per second at the outer pull radius (t = 1, farthest)
    UPROPERTY(EditAnywhere, Category = "Tornado|Damage")
    float MinDamagePerSecond = 2.f;

    UPROPERTY(EditAnywhere, Category = "Tornado|Damage")
    float DamageTickRate = 0.5f;

    // Player is ejected once this much total damage has accumulated — 2 health sections (2 x 25 HP)
    UPROPERTY(EditAnywhere, Category = "Tornado|Damage")
    float MaxTotalDamagePerCharacter = 50.f;

    // --- Physics ---

    // Eye radius — inner calm zone where centrifugal force counters inward suction (cm)
    UPROPERTY(EditAnywhere, Category = "Tornado|Physics")
    float ForceInnerRadius = 200.f;

    // Widest radius of the visible funnel / eye wall — peak swirl and updraft occur here (cm)
    UPROPERTY(EditAnywhere, Category = "Tornado|Physics")
    float TornadoBodyRadius = 600.f;

    // Outer pull zone — set to ~2x TornadoBodyRadius; OuterZone capsule should match (cm)
    UPROPERTY(EditAnywhere, Category = "Tornado|Physics")
    float ForceOuterRadius = 1200.f;

    // Peak rotational (swirl) force at the eye wall (cm/s²)
    UPROPERTY(EditAnywhere, Category = "Tornado|Physics")
    float MaxTangentialForce = 3500.f;

    // Peak inward suction force at the outer boundary (cm/s²)
    UPROPERTY(EditAnywhere, Category = "Tornado|Physics")
    float MaxInwardForce = 1500.f;

    // Peak updraft force inside the funnel body (cm/s²)
    UPROPERTY(EditAnywhere, Category = "Tornado|Physics")
    float MaxUpwardForce = 1000.f;

    // Outward launch speed when the player is ejected (cm/s)
    UPROPERTY(EditAnywhere, Category = "Tornado|Physics")
    float EjectionOutwardSpeed = 1500.f;

    // Upward launch speed when the player is ejected (cm/s)
    UPROPERTY(EditAnywhere, Category = "Tornado|Physics")
    float EjectionUpwardSpeed = 700.f;

private:
    UPROPERTY()
    TArray<ACharacter*> AffectedCharacters;

    // Cumulative damage dealt to each character during this pass through the tornado
    TMap<ACharacter*, float> CharacterDamageAccumulated;

    float CurrentSplineDistance = 0.f;
    bool  bReachedEnd = false;
    float DamageAccumulator = 0.f;

    void MoveAlongSpline(float DeltaTime);
    void ApplyForcesToCharacter(ACharacter* Character, float DeltaTime);
    void ApplyDamageTick(float DeltaTime);
    void EjectCharacter(ACharacter* Character);

    // Returns 0 at the tornado centre, 1 at ForceOuterRadius
    float GetNormalizedDistance(const FVector& Location) const;

    UFUNCTION()
    void OnOuterZoneBeginOverlap(
        UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOuterZoneEndOverlap(
        UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
