#pragma once

#include "CoreMinimal.h"
#include "Data/RuntimeGameplaySettingsTypes.h"
#include "GameFramework/SaveGame.h"
#include "RuntimeGameplaySettingsSaveGame.generated.h"

UCLASS()
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, SaveGame, Category="Runtime Gameplay Settings")
	int32 DataVersion = 1;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category="Runtime Gameplay Settings")
	FGuid SlotId;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category="Runtime Gameplay Settings")
	FRuntimeGameplaySettingsSnapshot Snapshot;
};
