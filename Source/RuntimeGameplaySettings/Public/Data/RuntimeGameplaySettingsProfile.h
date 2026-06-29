#pragma once

#include "CoreMinimal.h"
#include "Data/RuntimeGameplaySettingsTypes.h"
#include "Engine/DataAsset.h"
#include "RuntimeGameplaySettingsProfile.generated.h"

UCLASS(BlueprintType)
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	TArray<FRuntimeGameplaySettingsClassEntry> RuntimeGameplaySettingsEntries;
};
