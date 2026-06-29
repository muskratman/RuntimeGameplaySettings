#pragma once

#include "CoreMinimal.h"
#include "Data/RuntimeGameplaySettingsTypes.h"

class APlayerController;
class URuntimeGameplaySettingsProfile;

struct RUNTIMEGAMEPLAYSETTINGS_API FRuntimeGameplaySettingsSnapshotLibrary
{
	static FRuntimeGameplaySettingsSnapshot CaptureSnapshot(
		APlayerController* PlayerController,
		URuntimeGameplaySettingsProfile* Profile);

	static void ApplySnapshot(
		APlayerController* PlayerController,
		URuntimeGameplaySettingsProfile* Profile,
		const FRuntimeGameplaySettingsSnapshot& Snapshot);
};
