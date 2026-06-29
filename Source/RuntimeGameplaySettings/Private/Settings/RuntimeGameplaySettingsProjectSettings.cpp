#include "Settings/RuntimeGameplaySettingsProjectSettings.h"

#include "InputCoreTypes.h"
#include "UI/Elements/RuntimeGameplaySettingsButtonWidget.h"
#include "UI/Elements/RuntimeGameplaySettingsCheckboxWidget.h"
#include "UI/Elements/RuntimeGameplaySettingsCurveAssetWidget.h"
#include "UI/Elements/RuntimeGameplaySettingsCurveWidget.h"
#include "UI/Elements/RuntimeGameplaySettingsEnumWidget.h"
#include "UI/Elements/RuntimeGameplaySettingsFloatArrayElementWidget.h"
#include "UI/Elements/RuntimeGameplaySettingsFloatArrayWidget.h"
#include "UI/Elements/RuntimeGameplaySettingsParameterWidget.h"
#include "UI/Elements/RuntimeGameplaySettingsResetButtonWidget.h"
#include "UI/Elements/RuntimeGameplaySettingsVectorWidget.h"
#include "UI/RuntimeGameplaySettingsButtonsOverlayWidget.h"
#include "UI/RuntimeGameplaySettingsCategorySwitcherWidget.h"
#include "UI/RuntimeGameplaySettingsCategoryTabsWidget.h"
#include "UI/RuntimeGameplaySettingsComponentGroupWidget.h"
#include "UI/RuntimeGameplaySettingsSaveSlotPanelWidget.h"
#include "UI/RuntimeGameplaySettingsWidget.h"

URuntimeGameplaySettingsProjectSettings::URuntimeGameplaySettingsProjectSettings()
{
	ToggleKey = EKeys::U;
	WidgetClass = URuntimeGameplaySettingsWidget::StaticClass();
	SaveSlotPanelWidgetClass = URuntimeGameplaySettingsSaveSlotPanelWidget::StaticClass();
	ButtonsOverlayWidgetClass = URuntimeGameplaySettingsButtonsOverlayWidget::StaticClass();
	CategoryTabsWidgetClass = URuntimeGameplaySettingsCategoryTabsWidget::StaticClass();
	CategoryPageWidgetClass = URuntimeGameplaySettingsCategorySwitcherWidget::StaticClass();
	ComponentGroupWidgetClass = URuntimeGameplaySettingsComponentGroupWidget::StaticClass();
	CategoryButtonWidgetClass = URuntimeGameplaySettingsButtonWidget::StaticClass();
	BoolPropertyWidgetClass = URuntimeGameplaySettingsCheckboxWidget::StaticClass();
	NumberPropertyWidgetClass = URuntimeGameplaySettingsParameterWidget::StaticClass();
	EnumPropertyWidgetClass = URuntimeGameplaySettingsEnumWidget::StaticClass();
	VectorPropertyWidgetClass = URuntimeGameplaySettingsVectorWidget::StaticClass();
	ResetButtonWidgetClass = URuntimeGameplaySettingsResetButtonWidget::StaticClass();
	CurvePropertyWidgetClass = URuntimeGameplaySettingsCurveWidget::StaticClass();
	CurveAssetPropertyWidgetClass = URuntimeGameplaySettingsCurveAssetWidget::StaticClass();
	FloatArrayPropertyWidgetClass = URuntimeGameplaySettingsFloatArrayWidget::StaticClass();
	FloatArrayElementWidgetClass = URuntimeGameplaySettingsFloatArrayElementWidget::StaticClass();
}

FName URuntimeGameplaySettingsProjectSettings::GetCategoryName() const
{
	return TEXT("GameplaySettings");
}

FName URuntimeGameplaySettingsProjectSettings::GetSectionName() const
{
	return TEXT("RuntimeGameplaySettings");
}

bool URuntimeGameplaySettingsProjectSettings::SupportsAutoRegistration() const
{
	return false;
}
