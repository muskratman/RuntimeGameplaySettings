#include "Subsystems/RuntimeGameplaySettingsSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "SaveGame/RuntimeGameplaySettingsSaveGame.h"
#include "SaveGame/RuntimeGameplaySettingsSlotIndex.h"

namespace
{
const FString RuntimeGameplaySettingsSlotIndexSaveSlotName = TEXT("RuntimeGameplaySettings_Index");
const FString RuntimeGameplaySettingsSlotPrefix = TEXT("RuntimeGameplaySettings_Slot_");
constexpr int32 RuntimeGameplaySettingsSaveUserIndex = 0;
}

void URuntimeGameplaySettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadSlotIndex();
}

void URuntimeGameplaySettingsSubsystem::Deinitialize()
{
	SlotIndexSave = nullptr;
	ClearBaselineSnapshot();
	Super::Deinitialize();
}

const TArray<FRuntimeGameplaySettingsSlotDescriptor>& URuntimeGameplaySettingsSubsystem::GetAvailableSlots() const
{
	static const TArray<FRuntimeGameplaySettingsSlotDescriptor> EmptySlots;
	return SlotIndexSave ? SlotIndexSave->Slots : EmptySlots;
}

bool URuntimeGameplaySettingsSubsystem::HasCurrentSlot() const
{
	return SlotIndexSave
		&& SlotIndexSave->CurrentSlotId.IsValid()
		&& FindSlotDescriptorById(SlotIndexSave->CurrentSlotId) != nullptr;
}

FString URuntimeGameplaySettingsSubsystem::GetCurrentSlotDisplayName() const
{
	FRuntimeGameplaySettingsSlotDescriptor CurrentSlot;
	return TryGetCurrentSlot(CurrentSlot) ? CurrentSlot.DisplayName : FString();
}

bool URuntimeGameplaySettingsSubsystem::TryGetCurrentSlot(FRuntimeGameplaySettingsSlotDescriptor& OutSlot) const
{
	if (!HasCurrentSlot())
	{
		return false;
	}

	if (const FRuntimeGameplaySettingsSlotDescriptor* CurrentSlot = FindSlotDescriptorById(SlotIndexSave->CurrentSlotId))
	{
		OutSlot = *CurrentSlot;
		return true;
	}

	return false;
}

bool URuntimeGameplaySettingsSubsystem::TryLoadCurrentSnapshot(FRuntimeGameplaySettingsSnapshot& OutSnapshot) const
{
	if (!HasCurrentSlot())
	{
		return false;
	}

	if (const URuntimeGameplaySettingsSaveGame* LoadedSave = LoadSavePayload(SlotIndexSave->CurrentSlotId))
	{
		OutSnapshot = LoadedSave->Snapshot;
		return true;
	}

	return false;
}

bool URuntimeGameplaySettingsSubsystem::SaveCurrent(const FRuntimeGameplaySettingsSnapshot& Snapshot)
{
	if (!HasCurrentSlot())
	{
		return false;
	}

	FRuntimeGameplaySettingsSlotDescriptor* CurrentSlot = FindMutableSlotDescriptorById(SlotIndexSave->CurrentSlotId);
	if (!CurrentSlot || !WriteSavePayload(CurrentSlot->SlotId, Snapshot))
	{
		return false;
	}

	CurrentSlot->UpdatedAtUtc = FDateTime::UtcNow();
	return SaveSlotIndex();
}

bool URuntimeGameplaySettingsSubsystem::SaveAs(
	const FString& DesiredDisplayName,
	const FRuntimeGameplaySettingsSnapshot& Snapshot,
	FRuntimeGameplaySettingsSlotDescriptor& OutSlot)
{
	const FString SanitizedDisplayName = SanitizeDisplayName(DesiredDisplayName);
	if (SanitizedDisplayName.IsEmpty())
	{
		return false;
	}

	EnsureSlotIndexObject();

	FRuntimeGameplaySettingsSlotDescriptor* ExistingSlot = FindMutableSlotDescriptorByDisplayName(SanitizedDisplayName);
	const FDateTime Timestamp = FDateTime::UtcNow();
	const FGuid SlotId = ExistingSlot ? ExistingSlot->SlotId : FGuid::NewGuid();

	if (!WriteSavePayload(SlotId, Snapshot))
	{
		return false;
	}

	if (ExistingSlot)
	{
		ExistingSlot->DisplayName = SanitizedDisplayName;
		ExistingSlot->UpdatedAtUtc = Timestamp;
		OutSlot = *ExistingSlot;
	}
	else
	{
		FRuntimeGameplaySettingsSlotDescriptor NewSlot;
		NewSlot.SlotId = SlotId;
		NewSlot.DisplayName = SanitizedDisplayName;
		NewSlot.CreatedAtUtc = Timestamp;
		NewSlot.UpdatedAtUtc = Timestamp;
		SlotIndexSave->Slots.Add(NewSlot);
		OutSlot = NewSlot;
	}

	SlotIndexSave->CurrentSlotId = SlotId;
	return SaveSlotIndex();
}

bool URuntimeGameplaySettingsSubsystem::LoadSlot(const FGuid& SlotId, FRuntimeGameplaySettingsSnapshot& OutSnapshot)
{
	if (!SlotId.IsValid() || !FindSlotDescriptorById(SlotId))
	{
		return false;
	}

	if (const URuntimeGameplaySettingsSaveGame* LoadedSave = LoadSavePayload(SlotId))
	{
		OutSnapshot = LoadedSave->Snapshot;
		SlotIndexSave->CurrentSlotId = SlotId;
		return SaveSlotIndex();
	}

	return false;
}

bool URuntimeGameplaySettingsSubsystem::DeleteSlot(const FGuid& SlotId)
{
	if (!SlotId.IsValid())
	{
		return false;
	}

	FRuntimeGameplaySettingsSlotDescriptor* SlotToDelete = FindMutableSlotDescriptorById(SlotId);
	if (!SlotToDelete)
	{
		return false;
	}

	const FString SlotName = BuildSlotName(SlotId);
	if (UGameplayStatics::DoesSaveGameExist(SlotName, RuntimeGameplaySettingsSaveUserIndex)
		&& !UGameplayStatics::DeleteGameInSlot(SlotName, RuntimeGameplaySettingsSaveUserIndex))
	{
		return false;
	}

	SlotIndexSave->Slots.RemoveAll([&SlotId](const FRuntimeGameplaySettingsSlotDescriptor& Slot)
	{
		return Slot.SlotId == SlotId;
	});

	if (SlotIndexSave->CurrentSlotId == SlotId)
	{
		SlotIndexSave->CurrentSlotId.Invalidate();
	}

	return SaveSlotIndex();
}

bool URuntimeGameplaySettingsSubsystem::HasBaselineSnapshot() const
{
	return bHasBaselineSnapshot;
}

bool URuntimeGameplaySettingsSubsystem::TryGetBaselineSnapshot(FRuntimeGameplaySettingsSnapshot& OutSnapshot) const
{
	if (!bHasBaselineSnapshot)
	{
		return false;
	}

	OutSnapshot = BaselineSnapshot;
	return true;
}

void URuntimeGameplaySettingsSubsystem::SetBaselineSnapshot(const FRuntimeGameplaySettingsSnapshot& Snapshot)
{
	BaselineSnapshot = Snapshot;
	bHasBaselineSnapshot = true;
}

void URuntimeGameplaySettingsSubsystem::ClearBaselineSnapshot()
{
	BaselineSnapshot = FRuntimeGameplaySettingsSnapshot();
	bHasBaselineSnapshot = false;
}

void URuntimeGameplaySettingsSubsystem::EnsureSlotIndexObject()
{
	if (!SlotIndexSave)
	{
		SlotIndexSave = Cast<URuntimeGameplaySettingsSlotIndex>(
			UGameplayStatics::CreateSaveGameObject(URuntimeGameplaySettingsSlotIndex::StaticClass()));
	}
}

bool URuntimeGameplaySettingsSubsystem::LoadSlotIndex()
{
	if (UGameplayStatics::DoesSaveGameExist(RuntimeGameplaySettingsSlotIndexSaveSlotName, RuntimeGameplaySettingsSaveUserIndex))
	{
		SlotIndexSave = Cast<URuntimeGameplaySettingsSlotIndex>(
			UGameplayStatics::LoadGameFromSlot(RuntimeGameplaySettingsSlotIndexSaveSlotName, RuntimeGameplaySettingsSaveUserIndex));
	}

	EnsureSlotIndexObject();

	if (!SlotIndexSave)
	{
		return false;
	}

	if (SlotIndexSave->CurrentSlotId.IsValid() && !FindSlotDescriptorById(SlotIndexSave->CurrentSlotId))
	{
		SlotIndexSave->CurrentSlotId.Invalidate();
	}

	return true;
}

bool URuntimeGameplaySettingsSubsystem::SaveSlotIndex()
{
	EnsureSlotIndexObject();
	return SlotIndexSave
		&& UGameplayStatics::SaveGameToSlot(
			SlotIndexSave,
			RuntimeGameplaySettingsSlotIndexSaveSlotName,
			RuntimeGameplaySettingsSaveUserIndex);
}

FString URuntimeGameplaySettingsSubsystem::SanitizeDisplayName(const FString& InDisplayName) const
{
	return InDisplayName.TrimStartAndEnd();
}

FString URuntimeGameplaySettingsSubsystem::BuildSlotName(const FGuid& SlotId) const
{
	return RuntimeGameplaySettingsSlotPrefix + SlotId.ToString(EGuidFormats::DigitsWithHyphens);
}

URuntimeGameplaySettingsSaveGame* URuntimeGameplaySettingsSubsystem::CreateSavePayload() const
{
	return Cast<URuntimeGameplaySettingsSaveGame>(
		UGameplayStatics::CreateSaveGameObject(URuntimeGameplaySettingsSaveGame::StaticClass()));
}

URuntimeGameplaySettingsSaveGame* URuntimeGameplaySettingsSubsystem::LoadSavePayload(const FGuid& SlotId) const
{
	if (!SlotId.IsValid())
	{
		return nullptr;
	}

	const FString SlotName = BuildSlotName(SlotId);
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, RuntimeGameplaySettingsSaveUserIndex))
	{
		return nullptr;
	}

	return Cast<URuntimeGameplaySettingsSaveGame>(
		UGameplayStatics::LoadGameFromSlot(SlotName, RuntimeGameplaySettingsSaveUserIndex));
}

bool URuntimeGameplaySettingsSubsystem::WriteSavePayload(
	const FGuid& SlotId,
	const FRuntimeGameplaySettingsSnapshot& Snapshot)
{
	URuntimeGameplaySettingsSaveGame* SavePayload = CreateSavePayload();
	if (!SavePayload || !SlotId.IsValid())
	{
		return false;
	}

	SavePayload->SlotId = SlotId;
	SavePayload->Snapshot = Snapshot;
	return UGameplayStatics::SaveGameToSlot(
		SavePayload,
		BuildSlotName(SlotId),
		RuntimeGameplaySettingsSaveUserIndex);
}

const FRuntimeGameplaySettingsSlotDescriptor* URuntimeGameplaySettingsSubsystem::FindSlotDescriptorById(
	const FGuid& SlotId) const
{
	if (!SlotIndexSave || !SlotId.IsValid())
	{
		return nullptr;
	}

	return SlotIndexSave->Slots.FindByPredicate([&SlotId](const FRuntimeGameplaySettingsSlotDescriptor& Slot)
	{
		return Slot.SlotId == SlotId;
	});
}

FRuntimeGameplaySettingsSlotDescriptor* URuntimeGameplaySettingsSubsystem::FindMutableSlotDescriptorById(
	const FGuid& SlotId)
{
	if (!SlotIndexSave || !SlotId.IsValid())
	{
		return nullptr;
	}

	return SlotIndexSave->Slots.FindByPredicate([&SlotId](const FRuntimeGameplaySettingsSlotDescriptor& Slot)
	{
		return Slot.SlotId == SlotId;
	});
}

FRuntimeGameplaySettingsSlotDescriptor* URuntimeGameplaySettingsSubsystem::FindMutableSlotDescriptorByDisplayName(
	const FString& DisplayName)
{
	if (!SlotIndexSave)
	{
		return nullptr;
	}

	const FString SanitizedDisplayName = SanitizeDisplayName(DisplayName);
	if (SanitizedDisplayName.IsEmpty())
	{
		return nullptr;
	}

	return SlotIndexSave->Slots.FindByPredicate([&SanitizedDisplayName](const FRuntimeGameplaySettingsSlotDescriptor& Slot)
	{
		return Slot.DisplayName.Equals(SanitizedDisplayName, ESearchCase::IgnoreCase);
	});
}
