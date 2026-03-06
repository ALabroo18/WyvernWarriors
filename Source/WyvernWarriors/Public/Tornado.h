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

    // Outer capsule - triggers enter/exit detection
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tornado")
    UCapsuleComponent* OuterZone;

    // Inner capsule - used only for radius math, no collision
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tornado")
    UCapsuleComponent* InnerZone;

    UPROPERTY(EditAnywhere, Category = "Tornado|Movement")
    float MovementSpeed = 400.f;

    UPROPERTY(EditAnywhere, Category = "Tornado|Damage")
    float MaxDamagePerSecond = 80.f;

    UPROPERTY(EditAnywhere, Category = "Tornado|Damage")
    float MinDamagePerSecond = 5.f;

    // How often damage applies - actual hit = DPS * TickRate
    UPROPERTY(EditAnywhere, Category = "Tornado|Damage")
    float DamageTickRate = 0.2f;

    UPROPERTY(EditAnywhere, Category = "Tornado|Physics")
    float MaxTangentialForce = 3000.f;

    UPROPERTY(EditAnywhere, Category = "Tornado|Physics")
    float MaxInwardForce = 1200.f;

    UPROPERTY(EditAnywhere, Category = "Tornado|Physics")
    float MaxUpwardForce = 800.f;

    // Radius of the eye in cm
    UPROPERTY(EditAnywhere, Category = "Tornado|Physics")
    float ForceInnerRadius = 200.f;

    // Radius of the full influence zone in cm
    UPROPERTY(EditAnywhere, Category = "Tornado|Physics")
    float ForceOuterRadius = 1500.f;

private:
    // Characters currently inside OuterZone
    UPROPERTY()
    TArray<ACharacter*> AffectedCharacters;

    float CurrentSplineDistance = 0.f;
    bool  bReachedEnd = false;
    float DamageAccumulator = 0.f;

    void MoveAlongSpline(float DeltaTime);
    void ApplyForcesToCharacter(ACharacter* Character, float DeltaTime);
    void ApplyDamageTick(float DeltaTime);

    // Returns 0 at inner radius, 1 at outer radius
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