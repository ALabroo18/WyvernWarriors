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
	
	// Bool for if boss is attacking.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = true))
	bool bIsAttacking = false;
	
	// IBool for if boss is hovering.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = true))
	bool bIsHovering = false;
	
	// Bool for if boss is dazed.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = true))
	bool bIsDazed = false;
	
	// Bool for if boss is dead.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = true))
	bool bIsDefeated = false;

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
