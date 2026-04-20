#include "BossAnimInstance.h"
#include "Kismet/KismetMathLibrary.h"

/* Play dazed animation by changing dazed blend to 1.
 */
void UBossAnimInstance::PlayDazedAnimation()
{
	GetWorld()->GetTimerManager().SetTimer(
		DazedBlendTimerHandle,
		FTimerDelegate::CreateUObject(this, &UBossAnimInstance::ChangeDazedBlend, 1.f),
		0.05f,
		true
	);
}

/* Stop dazed animation by changing dazed blend to 0.
 */
void UBossAnimInstance::StopDazedAnimation()
{
	GetWorld()->GetTimerManager().SetTimer(
		DazedBlendTimerHandle,
		FTimerDelegate::CreateUObject(this, &UBossAnimInstance::ChangeDazedBlend, 0.f),
		0.05f,
		true
	);
}

/* Interp dazed blend alpha to target value, and stop dazed blend timer when nearly equal to target.
 * @param Target - float value that dazed blend alpha will become.
 */
void UBossAnimInstance::ChangeDazedBlend(float const Target)
{
	DazedBlendAlpha = UKismetMathLibrary::FInterpTo_Constant(
		DazedBlendAlpha, 
		Target, 
		GetWorld()->GetDeltaSeconds(), 
		DazedBlendInterpSpeed
	);

	UE_LOG(LogTemp, Log, TEXT("target blend alpha: %f"), Target);
	UE_LOG(LogTemp, Log, TEXT("dazed blend alpha: %f"), DazedBlendAlpha);
	
	if (DazedBlendAlpha == Target)
	{
		DazedBlendAlpha = Target;
		GetWorld()->GetTimerManager().ClearTimer(DazedBlendTimerHandle);
	}
}
