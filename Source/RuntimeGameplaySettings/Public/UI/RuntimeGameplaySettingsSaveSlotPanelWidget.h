#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/ComboBoxString.h"
#include "Data/RuntimeGameplaySettingsTypes.h"
#include "RuntimeGameplaySettingsSaveSlotPanelWidget.generated.h"

class UBorder;
class UButton;
class UComboBoxString;
class UEditableTextBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRuntimeGameplaySettingsSaveSlotPanelEvent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FRuntimeGameplaySettingsSaveSlotSelectionChangedEvent,
	FString,
	SelectedItem,
	ESelectInfo::Type,
	SelectionType);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FRuntimeGameplaySettingsSaveSlotNameChangedEvent,
	const FText&,
	InText);

UCLASS(Blueprintable)
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsSaveSlotPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="RuntimeGameplaySettings Save Slot")
	FRuntimeGameplaySettingsSaveSlotPanelEvent OnCloseRequested;

	UPROPERTY(BlueprintAssignable, Category="RuntimeGameplaySettings Save Slot")
	FRuntimeGameplaySettingsSaveSlotPanelEvent OnSaveAsRequested;

	UPROPERTY(BlueprintAssignable, Category="RuntimeGameplaySettings Save Slot")
	FRuntimeGameplaySettingsSaveSlotPanelEvent OnLoadRequested;

	UPROPERTY(BlueprintAssignable, Category="RuntimeGameplaySettings Save Slot")
	FRuntimeGameplaySettingsSaveSlotPanelEvent OnDeleteRequested;

	UPROPERTY(BlueprintAssignable, Category="RuntimeGameplaySettings Save Slot")
	FRuntimeGameplaySettingsSaveSlotSelectionChangedEvent OnSlotSelectionChanged;

	UPROPERTY(BlueprintAssignable, Category="RuntimeGameplaySettings Save Slot")
	FRuntimeGameplaySettingsSaveSlotNameChangedEvent OnSlotNameChanged;

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Save Slot")
	void SetPanelVisible(bool bVisible);

	UFUNCTION(BlueprintPure, Category="RuntimeGameplaySettings Save Slot")
	bool IsPanelVisible() const;

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Save Slot")
	void RefreshSlots(
		const TArray<FRuntimeGameplaySettingsSlotDescriptor>& Slots,
		const FString& PreferredSlotName,
		const FString& RequestedSlotName);

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Save Slot")
	void SetActionStates(bool bCanSaveAs, bool bCanLoad, bool bCanDelete);

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Save Slot")
	void SyncRequestedSlotName(const FString& SlotName);

	UFUNCTION(BlueprintPure, Category="RuntimeGameplaySettings Save Slot")
	FString GetRequestedSlotName() const;

	UFUNCTION(BlueprintPure, Category="RuntimeGameplaySettings Save Slot")
	FString GetSelectedSlotName() const;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UBorder> Border_SaveSlot;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_CloseSaveAs;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Butt_SaveAs;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Butt_Load;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Butt_Delete;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UComboBoxString> Combo_Slots;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> Editable_SlotName;

private:
	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleSaveAsClicked();

	UFUNCTION()
	void HandleLoadClicked();

	UFUNCTION()
	void HandleDeleteClicked();

	UFUNCTION()
	void HandleSlotSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleSlotNameChanged(const FText& InText);

	bool bIsSynchronizingSlots = false;

	void EnsureDefaultWidgetTree();
	void BuildDefaultWidgetTree();
};
