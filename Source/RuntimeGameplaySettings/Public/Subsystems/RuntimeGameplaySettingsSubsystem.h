#pragma once

#include "CoreMinimal.h"
#include "Data/RuntimeGameplaySettingsTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RuntimeGameplaySettingsSubsystem.generated.h"

class URuntimeGameplaySettingsSaveGame;
class URuntimeGameplaySettingsSlotIndex;

UCLASS()
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	const TArray<FRuntimeGameplaySettingsSlotDescriptor>& GetAvailableSlots() const;
	bool HasCurrentSlot() const;
	FString GetCurrentSlotDisplayName() const;
	bool TryGetCurrentSlot(FRuntimeGameplaySettingsSlotDescriptor& OutSlot) const;
	bool TryLoadCurrentSnapshot(FRuntimeGameplaySettingsSnapshot& OutSnapshot) const;
	bool SaveCurrent(const FRuntimeGameplaySettingsSnapshot& Snapshot);
	bool SaveAs(const FString& DesiredDisplayName, const FRuntimeGameplaySettingsSnapshot& Snapshot, FRuntimeGameplaySettingsSlotDescriptor& OutSlot);
	bool LoadSlot(const FGuid& SlotId, FRuntimeGameplaySettingsSnapshot& OutSnapshot);
	bool DeleteSlot(const FGuid& SlotId);

	bool HasBaselineSnapshot() const;
	bool TryGetBaselineSnapshot(FRuntimeGameplaySettingsSnapshot& OutSnapshot) const;
	void SetBaselineSnapshot(const FRuntimeGameplaySettingsSnapshot& Snapshot);
	void ClearBaselineSnapshot();

private:
	void EnsureSlotIndexObject();
	bool LoadSlotIndex();
	bool SaveSlotIndex();
	FString SanitizeDisplayName(const FString& InDisplayName) const;
	FString BuildSlotName(const FGuid& SlotId) const;
	URuntimeGameplaySettingsSaveGame* CreateSavePayload() const;
	URuntimeGameplaySettingsSaveGame* LoadSavePayload(const FGuid& SlotId) const;
	bool WriteSavePayload(const FGuid& SlotId, const FRuntimeGameplaySettingsSnapshot& Snapshot);
	const FRuntimeGameplaySettingsSlotDescriptor* FindSlotDescriptorById(const FGuid& SlotId) const;
	FRuntimeGameplaySettingsSlotDescriptor* FindMutableSlotDescriptorById(const FGuid& SlotId);
	FRuntimeGameplaySettingsSlotDescriptor* FindMutableSlotDescriptorByDisplayName(const FString& DisplayName);

	UPROPERTY(Transient)
	TObjectPtr<URuntimeGameplaySettingsSlotIndex> SlotIndexSave;

	UPROPERTY(Transient)
	FRuntimeGameplaySettingsSnapshot BaselineSnapshot;

	UPROPERTY(Transient)
	bool bHasBaselineSnapshot = false;
};
