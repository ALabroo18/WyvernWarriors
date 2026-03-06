#include "Tornado.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ATornado::ATornado()
{
    PrimaryActorTick.bCanEverTick = true;

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    SplinePath = CreateDefaultSubobject<USplineComponent>(TEXT("SplinePath"));
    SplinePath->SetupAttachment(Root);

    OuterZone = CreateDefaultSubobject<UCapsuleComponent>(TEXT("OuterZone"));
    OuterZone->SetupAttachment(Root);
    OuterZone->SetCapsuleSize(ForceOuterRadius, 1200.f);
    OuterZone->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    OuterZone->SetGenerateOverlapEvents(true);

    InnerZone = CreateDefaultSubobject<UCapsuleComponent>(TEXT("InnerZone"));
    InnerZone->SetupAttachment(Root);
    InnerZone->SetCapsuleSize(ForceInnerRadius, 1200.f);
    InnerZone->SetCollisionProfileName(TEXT("NoCollision"));
}

void ATornado::BeginPlay()
{
    Super::BeginPlay();

    OuterZone->OnComponentBeginOverlap.AddDynamic(this, &ATornado::OnOuterZoneBeginOverlap);
    OuterZone->OnComponentEndOverlap.AddDynamic(this, &ATornado::OnOuterZoneEndOverlap);

    if (SplinePath->GetSplineLength() > 0.f)
    {
        SetActorLocation(SplinePath->GetLocationAtDistanceAlongSpline(
            0.f, ESplineCoordinateSpace::World));
    }
}

void ATornado::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    AffectedCharacters.Empty();
    Super::EndPlay(EndPlayReason);
}

void ATornado::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // bReachedEnd guards against forces firing on the same frame Destroy() is called
    if (bReachedEnd) return;

    MoveAlongSpline(DeltaTime);

    for (ACharacter* Char : AffectedCharacters)
    {
        if (IsValid(Char))
            ApplyForcesToCharacter(Char, DeltaTime);
    }

    if (AffectedCharacters.Num() > 0)
        ApplyDamageTick(DeltaTime);
}

void ATornado::MoveAlongSpline(float DeltaTime)
{
    const float SplineLength = SplinePath->GetSplineLength();
    if (SplineLength <= 0.f) return;

    CurrentSplineDistance += MovementSpeed * DeltaTime;

    if (CurrentSplineDistance >= SplineLength)
    {
        bReachedEnd = true;
        Destroy();
        return;
    }

    SetActorLocationAndRotation(
        SplinePath->GetLocationAtDistanceAlongSpline(CurrentSplineDistance, ESplineCoordinateSpace::World),
        SplinePath->GetRotationAtDistanceAlongSpline(CurrentSplineDistance, ESplineCoordinateSpace::World));
}

void ATornado::ApplyForcesToCharacter(ACharacter* Character, float DeltaTime)
{
    UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
    if (!MoveComp) return;

    // Flatten to horizontal plane so height difference does not skew directions
    FVector ToCharacter = Character->GetActorLocation() - GetActorLocation();
    ToCharacter.Z = 0.f;
    if (ToCharacter.Size() < 1.f) return;

    const float NormalizedDist = GetNormalizedDistance(Character->GetActorLocation());
    const float ForceFalloff = 1.f - NormalizedDist; // 1 at center, 0 at edge

    // Inward direction toward tornado center
    const FVector InwardDir = -ToCharacter.GetSafeNormal();
    // Cross(Up, Inward) = clockwise tangent viewed from above
    const FVector TangentialDir = FVector::CrossProduct(FVector::UpVector, InwardDir).GetSafeNormal();

    // Swirl peaks at the eye wall (0.35 normalized), drops off inward and outward
    const float SwirlScale = FMath::Clamp(
        1.f - FMath::Abs(NormalizedDist - 0.35f) / 0.35f, 0.f, 1.f);

    // Suction strongest near eye wall, zero at outer edge
    const float SuctionScale = FMath::Clamp(1.f - NormalizedDist * 1.5f, 0.f, 1.f);

    const FVector TotalForce =
        TangentialDir * MaxTangentialForce * SwirlScale +
        InwardDir * MaxInwardForce * SuctionScale +
        FVector::UpVector * MaxUpwardForce * ForceFalloff;

    // Reduce gravity near center so upward force is not fighting full gravity
    MoveComp->GravityScale = FMath::Lerp(1.f, 0.1f, ForceFalloff);
    MoveComp->AddInputVector(TotalForce * DeltaTime, true);
}

void ATornado::ApplyDamageTick(float DeltaTime)
{
    DamageAccumulator += DeltaTime;
    if (DamageAccumulator < DamageTickRate) return;
    DamageAccumulator -= DamageTickRate; // subtract instead of zero to keep ticks regular

    for (ACharacter* Char : AffectedCharacters)
    {
        if (!IsValid(Char)) continue;

        const float NormalizedDist = GetNormalizedDistance(Char->GetActorLocation());
        const float Damage = FMath::Lerp(MaxDamagePerSecond, MinDamagePerSecond, NormalizedDist)
            * DamageTickRate;

        UGameplayStatics::ApplyDamage(Char, Damage, nullptr, this, UDamageType::StaticClass());
    }
}

float ATornado::GetNormalizedDistance(const FVector& Location) const
{
    FVector ToChar = Location - GetActorLocation();
    ToChar.Z = 0.f;
    return FMath::Clamp(
        (ToChar.Size() - ForceInnerRadius) / (ForceOuterRadius - ForceInnerRadius),
        0.f, 1.f);
}

void ATornado::OnOuterZoneBeginOverlap(
    UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character && !AffectedCharacters.Contains(Character))
        AffectedCharacters.Add(Character);
}

void ATornado::OnOuterZoneEndOverlap(
    UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character)
    {
        AffectedCharacters.Remove(Character);

        // Restore gravity on exit
        if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
            MoveComp->GravityScale = 1.f;
    }
}