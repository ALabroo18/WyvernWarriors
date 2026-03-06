#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ActivatableInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UActivatableInterface : public UInterface
{
	GENERATED_BODY()
};

class IActivatableInterface
{
	GENERATED_BODY()

public:
	// Sets the activeness of the object
	virtual void SetActiveness(bool const bIsActive); 
};
