#pragma once

#include "CoreMinimal.h"
#include "Data/RuntimeGameplaySettingsTypes.h"

class FProperty;
class UActorComponent;

struct RUNTIMEGAMEPLAYSETTINGS_API FRuntimeGameplaySettingsPropertyAccess
{
	static ERuntimeGameplaySettingsValueType GetSupportedValueType(const FProperty* Property, FString* OutEnumPath = nullptr);
	static bool IsSupportedEditableProperty(const FProperty* Property);
	static FRuntimeGameplaySettingsDiscoveredProperty BuildDiscoveredProperty(
		const FProperty* Property,
		FName ComponentName = NAME_None,
		UClass* ComponentClass = nullptr);
	static FRuntimeGameplaySettingsPropertyEntry BuildPropertyEntry(const FRuntimeGameplaySettingsDiscoveredProperty& DiscoveredProperty);
	static FProperty* FindRuntimeProperty(UObject* TargetObject, const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry);
	static bool ReadValue(UObject* TargetObject, const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry, FRuntimeGameplaySettingsValue& OutValue);
	static bool WriteValue(UObject* TargetObject, const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry, const FRuntimeGameplaySettingsValue& Value);
	static TArray<FString> BuildEnumOptions(const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry);
	static bool TryResolveEnumValueFromDisplayName(const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry, const FString& DisplayName, int32& OutValue);
	static FString GetEnumDisplayName(const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry, int32 Value);
	static TArray<FName> GetEffectivePropertyPath(const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry);
	static FString BuildPropertyPathString(const TArray<FName>& PropertyPath, FName FallbackPropertyName = NAME_None);

private:
	static const UEnum* ResolveEnum(const FString& EnumPath);
	static UObject* ResolvePropertyOwner(UObject* TargetObject, const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry);
	static bool ResolveRuntimeProperty(
		UObject* TargetObject,
		const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry,
		FProperty*& OutProperty,
		void*& OutValuePtr,
		FString* OutRuntimeEnumPath = nullptr,
		UObject** OutPropertyOwner = nullptr);
	static void NotifyRuntimePropertyChanged(UObject* PropertyOwner, const FProperty* ChangedProperty);
	static FName NormalizeComponentName(FName ComponentName);
};
