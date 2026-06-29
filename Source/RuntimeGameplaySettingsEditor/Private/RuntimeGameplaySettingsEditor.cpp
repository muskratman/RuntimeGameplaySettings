#include "RuntimeGameplaySettingsEditor.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "RuntimeGameplaySettingsClassEntryCustomization.h"
#include "RuntimeGameplaySettingsPropertyEntryCustomization.h"
#include "RuntimeGameplaySettingsProfileCustomization.h"
#include "Settings/RuntimeGameplaySettingsProjectSettings.h"
#include "ISettingsModule.h"

#define LOCTEXT_NAMESPACE "FRuntimeGameplaySettingsEditorModule"

void FRuntimeGameplaySettingsEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyEditorModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyEditorModule.RegisterCustomClassLayout(
		"RuntimeGameplaySettingsProfile",
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FRuntimeGameplaySettingsProfileCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(
		"RuntimeGameplaySettingsClassEntry",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(
			&FRuntimeGameplaySettingsClassEntryCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(
		"RuntimeGameplaySettingsPropertyEntry",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(
			&FRuntimeGameplaySettingsPropertyEntryCustomization::MakeInstance));
	PropertyEditorModule.NotifyCustomizationModuleChanged();

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings(
			"Project",
			"GameplaySettings",
			"RuntimeGameplaySettings",
			LOCTEXT("RuntimeGameplaySettingsSettingsName", "Runtime Gameplay Settings"),
			LOCTEXT("RuntimeGameplaySettingsSettingsDescription", "Configure Runtime Gameplay Settings plugin behavior."),
			GetMutableDefault<URuntimeGameplaySettingsProjectSettings>());
	}
}

void FRuntimeGameplaySettingsEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("Settings"))
	{
		ISettingsModule& SettingsModule = FModuleManager::GetModuleChecked<ISettingsModule>("Settings");
		SettingsModule.UnregisterSettings(
			"Project",
			"GameplaySettings",
			"RuntimeGameplaySettings");
	}

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyEditorModule =
			FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.UnregisterCustomClassLayout("RuntimeGameplaySettingsProfile");
		PropertyEditorModule.UnregisterCustomPropertyTypeLayout("RuntimeGameplaySettingsClassEntry");
		PropertyEditorModule.UnregisterCustomPropertyTypeLayout("RuntimeGameplaySettingsPropertyEntry");
		PropertyEditorModule.NotifyCustomizationModuleChanged();
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRuntimeGameplaySettingsEditorModule, RuntimeGameplaySettingsEditor)
