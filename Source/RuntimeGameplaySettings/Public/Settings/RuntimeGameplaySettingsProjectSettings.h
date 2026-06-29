#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InputCoreTypes.h"
#include "RuntimeGameplaySettingsProjectSettings.generated.h"

class URuntimeGameplaySettingsProfile;
class URuntimeGameplaySettingsWidget;
class URuntimeGameplaySettingsSaveSlotPanelWidget;
class URuntimeGameplaySettingsButtonsOverlayWidget;
class URuntimeGameplaySettingsCategoryTabsWidget;
class URuntimeGameplaySettingsCategorySwitcherWidget;
class URuntimeGameplaySettingsComponentGroupWidget;
class URuntimeGameplaySettingsButtonWidget;
class URuntimeGameplaySettingsCheckboxWidget;
class URuntimeGameplaySettingsParameterWidget;
class URuntimeGameplaySettingsEnumWidget;
class URuntimeGameplaySettingsVectorWidget;
class URuntimeGameplaySettingsResetButtonWidget;
class URuntimeGameplaySettingsCurveWidget;
class URuntimeGameplaySettingsCurveAssetWidget;
class URuntimeGameplaySettingsFloatArrayWidget;
class URuntimeGameplaySettingsFloatArrayElementWidget;

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

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings")
	bool bPauseGameWhileOpen = true;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|UI")
	TSoftClassPtr<URuntimeGameplaySettingsWidget> WidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|UI")
	TSoftClassPtr<URuntimeGameplaySettingsSaveSlotPanelWidget> SaveSlotPanelWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|UI")
	TSoftClassPtr<URuntimeGameplaySettingsButtonsOverlayWidget> ButtonsOverlayWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|UI")
	TSoftClassPtr<URuntimeGameplaySettingsCategoryTabsWidget> CategoryTabsWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|UI")
	TSoftClassPtr<URuntimeGameplaySettingsCategorySwitcherWidget> CategoryPageWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|UI")
	TSoftClassPtr<URuntimeGameplaySettingsComponentGroupWidget> ComponentGroupWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|UI|Elements")
	TSoftClassPtr<URuntimeGameplaySettingsButtonWidget> CategoryButtonWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|UI|Elements")
	TSoftClassPtr<URuntimeGameplaySettingsCheckboxWidget> BoolPropertyWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|UI|Elements")
	TSoftClassPtr<URuntimeGameplaySettingsParameterWidget> NumberPropertyWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|UI|Elements")
	TSoftClassPtr<URuntimeGameplaySettingsEnumWidget> EnumPropertyWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|UI|Elements")
	TSoftClassPtr<URuntimeGameplaySettingsVectorWidget> VectorPropertyWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|UI|Elements")
	TSoftClassPtr<URuntimeGameplaySettingsResetButtonWidget> ResetButtonWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|UI|Elements")
	TSoftClassPtr<URuntimeGameplaySettingsCurveWidget> CurvePropertyWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|UI|Elements")
	TSoftClassPtr<URuntimeGameplaySettingsCurveAssetWidget> CurveAssetPropertyWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|UI|Elements")
	TSoftClassPtr<URuntimeGameplaySettingsFloatArrayWidget> FloatArrayPropertyWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|UI|Elements")
	TSoftClassPtr<URuntimeGameplaySettingsFloatArrayElementWidget> FloatArrayElementWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|Input")
	bool bEnableToggleHotkey = true;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|Input")
	FKey ToggleKey;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|Input")
	bool bConsumeInput = true;

	UPROPERTY(Config, EditAnywhere, Category="Runtime Gameplay Settings|Input")
	bool bExecuteWhenPaused = true;

	virtual FName GetCategoryName() const override;
	virtual FName GetSectionName() const override;
	virtual bool SupportsAutoRegistration() const override;
};
