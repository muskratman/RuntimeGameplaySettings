#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InputCoreTypes.h"
#include "RuntimeGameplaySettingsProjectSettings.generated.h"

class URuntimeGameplaySettingsProfile;
class URuntimeGameplaySettingsWidget;

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Runtime Gameplay Settings"))
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	URuntimeGameplaySettingsProjectSettings();

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings")
	bool bEnableRuntimeGameplaySettings = false;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings")
	TSoftObjectPtr<URuntimeGameplaySettingsProfile> DefaultProfile;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|UI")
	TSoftClassPtr<URuntimeGameplaySettingsWidget> WidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|Input")
	bool bEnableToggleHotkey = true;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|Input")
	FKey ToggleKey;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|Input")
	bool bConsumeInput = true;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|Input")
	bool bExecuteWhenPaused = true;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|UI")
	bool bPauseGameWhileOpen = true;

	virtual FName GetCategoryName() const override;
	virtual FName GetSectionName() const override;
	virtual bool SupportsAutoRegistration() const override;
};
