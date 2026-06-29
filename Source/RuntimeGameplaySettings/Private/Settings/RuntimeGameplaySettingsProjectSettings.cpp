#include "Settings/RuntimeGameplaySettingsProjectSettings.h"

#include "InputCoreTypes.h"
#include "UI/RuntimeGameplaySettingsWidget.h"

URuntimeGameplaySettingsProjectSettings::URuntimeGameplaySettingsProjectSettings()
{
	ToggleKey = EKeys::U;
	WidgetClass = URuntimeGameplaySettingsWidget::StaticClass();
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
