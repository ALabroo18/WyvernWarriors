#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BossAnimInstance.generated.h"

UCLASS()
class UBossAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	// Play dazed animation by changing dazed blend to 1.
	UFUNCTION(Category = "Animation")
	void PlayDazedAnimation();
	
	// Stop dazed animation by changing dazed blend to 0.
	UFUNCTION(Category = "Animation")
	void StopDazedAnimation();

private:
	// Changes dazed blend alpha to target.
	UFUNCTION(Category = "Animation")
	void ChangeDazedBlend(float Target);
	
	// Alpha for the blend between default and dazed anims.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = true))
	float DazedBlendAlpha;
	
	// Timer handle for the daze blend changing function.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = true))
	FTimerHandle DazedBlendTimerHandle;
};
