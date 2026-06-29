#pragma once

#include "CoreMinimal.h"

class APlayerController;

struct RUNTIMEGAMEPLAYSETTINGS_API FRuntimeGameplaySettingsTargetResolver
{
	static TArray<UObject*> ResolveTargets(APlayerController* PlayerController, UClass* TargetClass);
};
