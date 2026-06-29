#pragma once

#include "Modules/ModuleManager.h"

class FRuntimeGameplaySettingsEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
