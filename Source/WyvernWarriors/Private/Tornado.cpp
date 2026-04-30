#include "Tornado.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"

ATornado::ATornado()
{
    PrimaryActorTick.bCanEverTick = true;

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    SplinePath = CreateDefaultSubobject<USplineComponent>(TEXT("SplinePath"));
    SplinePath->SetupAttachment(Root);
    SplinePath->SetAbsolute(true, true, true);

    // Pull zone: ForceOuterRadius = 2x TornadoBodyRadius by design
    OuterZone = CreateDefaultSubobject<UCapsuleComponent>(TEXT("OuterZone"));
    OuterZone->SetupAttachment(Root);
    OuterZone->SetCapsuleSize(ForceOuterRadius, 1200.f);
    OuterZone->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    OuterZone->SetGenerateOverlapEvents(true);

    // Funnel body reference shape — sized to TornadoBodyRadius, no gameplay collision
    InnerZone = CreateDefaultSubobject<UCapsuleComponent>(TEXT("InnerZone"));
    InnerZone->SetupAttachment(Root);
    InnerZone->SetCapsuleSize(TornadoBodyRadius, 1200.f);
    InnerZone->SetCollisionProfileName(TEXT("NoCollision"));
}

void ATornado::BeginPlay()
{
    Super::BeginPlay();

    OuterZone->OnComponentBeginOverlap.AddDynamic(this, &ATornado::OnOuterZoneBeginOverlap);
    OuterZone->OnComponentEndOverlap.AddDynamic(this, &ATornado::OnOuterZoneEndOverlap);

    TArray<UNiagaraComponent*> NiagaraComps;
    GetComponents<UNiagaraComponent>(NiagaraComps);
    for (UNiagaraComponent* NC : NiagaraComps)
    {
        if (NC)
        {
            NC->Activate(true);
        }
    }

    if (SplinePath->GetSplineLength() > 0.f)
    {
        SetActorLocation(SplinePath->GetLocationAtDistanceAlongSpline(
            0.f, ESplineCoordinateSpace::World));
    }
}

void ATornado::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    AffectedCharacters.Empty();
    CharacterDamageAccumulated.Empty();
    Super::EndPlay(EndPlayReason);
}

void ATornado::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

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

    FVector ToCharacter = Character->GetActorLocation() - GetActorLocation();
    ToCharacter.Z = 0.f;
    if (ToCharacter.Size() < 1.f) return;

    const float t     = GetNormalizedDistance(Character->GetActorLocation()); // 0=centre, 1=outer edge
    const float tEye  = ForceInnerRadius  / FMath::Max(ForceOuterRadius, 1.f);
    const float tBody = TornadoBodyRadius / FMath::Max(ForceOuterRadius, 1.f);

    const FVector InwardDir     = -ToCharacter.GetSafeNormal();
    // Counter-clockwise swirl viewed from above (flip cross order for CW)
    const FVector TangentialDir = FVector::CrossProduct(FVector::UpVector, InwardDir).GetSafeNormal();

    // Tangential (swirl): peaks at eye wall (tBody), strong through the outer band, very weak inside the eye
    float TangentialScale;
    if (t <= tBody)
        TangentialScale = FMath::GetMappedRangeValueClamped(FVector2D(0.f, tBody), FVector2D(0.15f, 1.f), t);
    else
        TangentialScale = FMath::GetMappedRangeValueClamped(FVector2D(tBody, 1.f), FVector2D(1.f, 0.45f), t);

    // Inward suction: zero inside the eye (centrifugal force counteracts it), ramps up through the
    // wall and peaks at the outer edge — this is what draws the player into the funnel
    const float InwardScale = t > tEye
        ? FMath::Clamp((t - tEye) / (1.f - tEye), 0.f, 1.f)
        : 0.f;

    // Updraft: strongest at the eye and eye wall, tapers to zero at the outer pull boundary
    float UpwardScale;
    if (t <= tBody)
        UpwardScale = FMath::GetMappedRangeValueClamped(FVector2D(0.f, tBody), FVector2D(1.f, 0.55f), t);
    else
        UpwardScale = FMath::GetMappedRangeValueClamped(FVector2D(tBody, 1.f), FVector2D(0.55f, 0.f), t);

    const FVector TotalForce =
        TangentialDir   * MaxTangentialForce * TangentialScale +
        InwardDir       * MaxInwardForce     * InwardScale     +
        FVector::UpVector * MaxUpwardForce   * UpwardScale;

    // Gravity suppressed inside the funnel body where the updraft dominates
    MoveComp->GravityScale = t < tBody
        ? FMath::GetMappedRangeValueClamped(FVector2D(0.f, tBody), FVector2D(0.05f, 0.5f), t)
        : 1.f;

    MoveComp->AddInputVector(TotalForce * DeltaTime, true);
}

void ATornado::ApplyDamageTick(float DeltaTime)
{
    DamageAccumulator += DeltaTime;
    if (DamageAccumulator < DamageTickRate) return;
    DamageAccumulator -= DamageTickRate;

    TArray<ACharacter*> ToEject;

    for (ACharacter* Char : AffectedCharacters)
    {
        if (!IsValid(Char)) continue;

        float& AccumDamage = CharacterDamageAccumulated.FindOrAdd(Char);

        // Already hit the cap on a previous tick — eject now
        if (AccumDamage >= MaxTotalDamagePerCharacter)
        {
            ToEject.Add(Char);
            continue;
        }

        const float t = GetNormalizedDistance(Char->GetActorLocation());
        const float RawDamage = FMath::Lerp(MaxDamagePerSecond, MinDamagePerSecond, t) * DamageTickRate;

        // Clamp so cumulative damage never exceeds the cap — the player cannot be killed by the tornado
        const float DamageToApply = FMath::Min(RawDamage, MaxTotalDamagePerCharacter - AccumDamage);
        AccumDamage += DamageToApply;

        UGameplayStatics::ApplyDamage(Char, DamageToApply, nullptr, this, UDamageType::StaticClass());

        if (AccumDamage >= MaxTotalDamagePerCharacter)
            ToEject.Add(Char);
    }

    // Eject after the loop to avoid invalidating the iterator
    for (ACharacter* Char : ToEject)
        EjectCharacter(Char);
}

void ATornado::EjectCharacter(ACharacter* Character)
{
    FVector OutDir = Character->GetActorLocation() - GetActorLocation();
    OutDir.Z = 0.f;

    // Fallback if the character is somehow exactly at the centre
    if (OutDir.SizeSquared() < 1.f)
        OutDir = FVector(FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f), 0.f);

    OutDir.Normalize();

    Character->LaunchCharacter(
        OutDir * EjectionOutwardSpeed + FVector::UpVector * EjectionUpwardSpeed,
        true, true);

    AffectedCharacters.Remove(Character);
    CharacterDamageAccumulated.Remove(Character);

    if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
        MoveComp->GravityScale = 1.f;
}

float ATornado::GetNormalizedDistance(const FVector& Location) const
{
    FVector ToChar = Location - GetActorLocation();
    ToChar.Z = 0.f;
    return FMath::Clamp(ToChar.Size() / FMath::Max(ForceOuterRadius, 1.f), 0.f, 1.f);
}

void ATornado::OnOuterZoneBeginOverlap(
    UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character && !AffectedCharacters.Contains(Character))
    {
        AffectedCharacters.Add(Character);
        // Re-entry starts the damage counter fresh
        CharacterDamageAccumulated.Remove(Character);
    }
}

void ATornado::OnOuterZoneEndOverlap(
    UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character)
    {
        AffectedCharacters.Remove(Character);
        CharacterDamageAccumulated.Remove(Character);

        if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
            MoveComp->GravityScale = 1.f;
    }
}
