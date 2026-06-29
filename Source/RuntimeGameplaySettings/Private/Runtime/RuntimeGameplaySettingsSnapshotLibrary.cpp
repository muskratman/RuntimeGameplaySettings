#include "Runtime/RuntimeGameplaySettingsSnapshotLibrary.h"

#include "Data/RuntimeGameplaySettingsProfile.h"
#include "GameFramework/PlayerController.h"
#include "Runtime/RuntimeGameplaySettingsPropertyAccess.h"
#include "Runtime/RuntimeGameplaySettingsTargetResolver.h"

DEFINE_LOG_CATEGORY_STATIC(LogRuntimeGameplaySettingsSnapshot, Log, All);

FRuntimeGameplaySettingsSnapshot FRuntimeGameplaySettingsSnapshotLibrary::CaptureSnapshot(
	APlayerController* PlayerController,
	URuntimeGameplaySettingsProfile* Profile)
{
	FRuntimeGameplaySettingsSnapshot Snapshot;
	Snapshot.ProfilePath = FSoftObjectPath(Profile);
	if (!PlayerController || !Profile)
	{
		return Snapshot;
	}

	for (const FRuntimeGameplaySettingsClassEntry& ClassEntry : Profile->RuntimeGameplaySettingsEntries)
	{
		UClass* TargetClass = ClassEntry.TargetClass.LoadSynchronous();
		if (!TargetClass)
		{
			continue;
		}

		TArray<UObject*> Targets = FRuntimeGameplaySettingsTargetResolver::ResolveTargets(PlayerController, TargetClass);
		if (Targets.Num() == 0)
		{
			continue;
		}
		if (Targets.Num() > 1)
		{
			UE_LOG(
				LogRuntimeGameplaySettingsSnapshot,
				Warning,
				TEXT("Multiple targets matched class %s; capturing first target only."),
				*GetNameSafe(TargetClass));
		}

		UObject* Target = Targets[0];
		FRuntimeGameplaySettingsObjectSnapshot ObjectSnapshot;
		ObjectSnapshot.TargetClassPath = FSoftClassPath(TargetClass);

		for (const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry : ClassEntry.Properties)
		{
			const FString PropertyPathString =
				FRuntimeGameplaySettingsPropertyAccess::BuildPropertyPathString(
					PropertyEntry.PropertyPath,
					PropertyEntry.PropertyName);
			FRuntimeGameplaySettingsValue Value;
			if (!FRuntimeGameplaySettingsPropertyAccess::ReadValue(Target, PropertyEntry, Value))
			{
				UE_LOG(
					LogRuntimeGameplaySettingsSnapshot,
					Warning,
					TEXT("Failed to capture property %s on %s."),
					*PropertyPathString,
					*GetNameSafe(Target));
				continue;
			}

			FRuntimeGameplaySettingsPropertySnapshot PropertySnapshot;
			PropertySnapshot.bIsComponentProperty = PropertyEntry.bIsComponentProperty;
			PropertySnapshot.ComponentName = PropertyEntry.ComponentName;
			PropertySnapshot.ComponentClassPath = PropertyEntry.ComponentClassPath;
			PropertySnapshot.PropertyName = PropertyEntry.PropertyName;
			PropertySnapshot.PropertyPath = PropertyEntry.PropertyPath;
			PropertySnapshot.ValueType = PropertyEntry.ValueType;
			PropertySnapshot.EnumPath = PropertyEntry.EnumPath;
			PropertySnapshot.Value = Value;
			ObjectSnapshot.Properties.Add(PropertySnapshot);
		}

		if (ObjectSnapshot.Properties.Num() > 0)
		{
			Snapshot.Objects.Add(ObjectSnapshot);
		}
	}

	return Snapshot;
}

void FRuntimeGameplaySettingsSnapshotLibrary::ApplySnapshot(
	APlayerController* PlayerController,
	URuntimeGameplaySettingsProfile* Profile,
	const FRuntimeGameplaySettingsSnapshot& Snapshot)
{
	if (!PlayerController || !Profile)
	{
		return;
	}

	for (const FRuntimeGameplaySettingsObjectSnapshot& ObjectSnapshot : Snapshot.Objects)
	{
		UClass* TargetClass = ObjectSnapshot.TargetClassPath.TryLoadClass<UObject>();
		if (!TargetClass)
		{
			continue;
		}

		TArray<UObject*> Targets = FRuntimeGameplaySettingsTargetResolver::ResolveTargets(PlayerController, TargetClass);
		if (Targets.Num() == 0)
		{
			continue;
		}
		if (Targets.Num() > 1)
		{
			UE_LOG(
				LogRuntimeGameplaySettingsSnapshot,
				Warning,
				TEXT("Multiple targets matched class %s; applying to all matched targets."),
				*GetNameSafe(TargetClass));
		}

		for (UObject* Target : Targets)
		{
			for (const FRuntimeGameplaySettingsPropertySnapshot& PropertySnapshot : ObjectSnapshot.Properties)
			{
				FRuntimeGameplaySettingsPropertyEntry PropertyEntry;
				PropertyEntry.bIsComponentProperty = PropertySnapshot.bIsComponentProperty;
				PropertyEntry.ComponentName = PropertySnapshot.ComponentName;
				PropertyEntry.ComponentClassPath = PropertySnapshot.ComponentClassPath;
				PropertyEntry.PropertyName = PropertySnapshot.PropertyName;
				PropertyEntry.PropertyPath = PropertySnapshot.PropertyPath;
				PropertyEntry.ValueType = PropertySnapshot.ValueType;
				PropertyEntry.EnumPath = PropertySnapshot.EnumPath;
				const FString PropertyPathString =
					FRuntimeGameplaySettingsPropertyAccess::BuildPropertyPathString(
						PropertyEntry.PropertyPath,
						PropertyEntry.PropertyName);

				if (!FRuntimeGameplaySettingsPropertyAccess::WriteValue(Target, PropertyEntry, PropertySnapshot.Value))
				{
					UE_LOG(
						LogRuntimeGameplaySettingsSnapshot,
						Warning,
						TEXT("Failed to apply property %s on %s."),
						*PropertyPathString,
						*GetNameSafe(Target));
				}
			}
		}
	}
}
