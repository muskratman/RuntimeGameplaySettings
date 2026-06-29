#pragma once

#include "CoreMinimal.h"
#include "Data/RuntimeGameplaySettingsTypes.h"
#include "GameFramework/SaveGame.h"
#include "RuntimeGameplaySettingsSlotIndex.generated.h"

UCLASS()
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsSlotIndex : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, SaveGame, Category="Runtime Gameplay Settings")
	TArray<FRuntimeGameplaySettingsSlotDescriptor> Slots;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category="Runtime Gameplay Settings")
	FGuid CurrentSlotId;
};
