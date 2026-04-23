#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BossAnimInstance.generated.h"

UCLASS()
class UBossAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	// Changes dazed blend alpha to target.
	UFUNCTION(Category = "Animation")
	void ChangeDazedBlend(float Target);

private:
	// Changes dazed blend alpha to target.
	UFUNCTION(Category = "Animation")
	void ChangeDazedBlendBackend(float Target);
	
	// Alpha for the blend between default and dazed anims.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = true))
	float DazedBlendAlpha;

	// Interpolation speed for dazed blend alpha.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = true))
	float DazedBlendInterpSpeed = 5.f;
	
	// Timer handle for the daze blend changing function.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = true))
	FTimerHandle DazedBlendTimerHandle;
};
