#include "BossAnimInstance.h"
#include "Kismet/KismetMathLibrary.h"


/* Change dazed blend alpha to target.
 */
void UBossAnimInstance::ChangeDazedBlend(float Target)
{
	GetWorld()->GetTimerManager().SetTimer(
		DazedBlendTimerHandle,
		FTimerDelegate::CreateUObject(this, &UBossAnimInstance::ChangeDazedBlendBackend, Target),
		0.05f,
		true
	);
}

/* Interp dazed blend alpha to target value, and stop dazed blend timer when nearly equal to target.
 * @param Target - float value that dazed blend alpha will become.
 */
void UBossAnimInstance::ChangeDazedBlendBackend(float const Target)
{
	DazedBlendAlpha = UKismetMathLibrary::FInterpTo_Constant(
		DazedBlendAlpha, 
		Target, 
		GetWorld()->GetDeltaSeconds(), 
		DazedBlendInterpSpeed
	);
	
	if (DazedBlendAlpha == Target)
	{
		DazedBlendAlpha = Target;
		GetWorld()->GetTimerManager().ClearTimer(DazedBlendTimerHandle);
	}
}
