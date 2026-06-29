#pragma once

#include "Modules/ModuleManager.h"

class FRuntimeGameplaySettingsModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
