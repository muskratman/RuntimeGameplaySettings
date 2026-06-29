#pragma once

#include "Blueprint/UserWidget.h"
#include "Data/RuntimeGameplaySettingsTypes.h"
#include "RuntimeGameplaySettingsWidget.generated.h"

class APlayerController;
class URuntimeGameplaySettingsProfile;
class URuntimeGameplaySettingsSubsystem;
class URuntimeGameplaySettingsButtonWidget;
class URuntimeGameplaySettingsButtonsOverlayWidget;
class URuntimeGameplaySettingsCategorySwitcherWidget;
class URuntimeGameplaySettingsCategoryTabsWidget;
class URuntimeGameplaySettingsCheckboxWidget;
class URuntimeGameplaySettingsEnumWidget;
class URuntimeGameplaySettingsParameterWidget;
class URuntimeGameplaySettingsSaveSlotPanelWidget;
class URuntimeGameplaySettingsValueWidgetBase;
class URuntimeGameplaySettingsVectorWidget;
class UTextBlock;
class UVerticalBox;
class UWidget;
class UWidgetSwitcher;

UCLASS()
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	URuntimeGameplaySettingsWidget(const FObjectInitializer& ObjectInitializer);

	FSimpleDelegate OnRequestClose;

	void InitializeRuntimeGameplaySettings(
		APlayerController* InPlayerController,
		URuntimeGameplaySettingsProfile* InProfile,
		URuntimeGameplaySettingsSubsystem* InSubsystem);

	bool ApplyRuntimeGameplaySettingsToTargets();
	void CloseRuntimeGameplaySettingsWidget();

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(EditDefaultsOnly, Category="RuntimeGameplaySettings|UI")
	TSubclassOf<URuntimeGameplaySettingsButtonWidget> CategoryButtonWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="RuntimeGameplaySettings|UI")
	TSubclassOf<URuntimeGameplaySettingsCategorySwitcherWidget> CategoryPageWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="RuntimeGameplaySettings|UI|Elements")
	TSubclassOf<URuntimeGameplaySettingsCheckboxWidget> BoolPropertyWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="RuntimeGameplaySettings|UI|Elements")
	TSubclassOf<URuntimeGameplaySettingsParameterWidget> NumberPropertyWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="RuntimeGameplaySettings|UI|Elements")
	TSubclassOf<URuntimeGameplaySettingsEnumWidget> EnumPropertyWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="RuntimeGameplaySettings|UI|Elements")
	TSubclassOf<URuntimeGameplaySettingsVectorWidget> VectorPropertyWidgetClass;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<URuntimeGameplaySettingsSaveSlotPanelWidget> SaveSlotPanelWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<URuntimeGameplaySettingsButtonsOverlayWidget> ButtonsOverlayWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<URuntimeGameplaySettingsCategoryTabsWidget> CategoryTabsWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWidgetSwitcher> Switcher_DeveloperSettings;

private:
	struct FRuntimeGameplaySettingsRowState
	{
		TWeakObjectPtr<UObject> TargetObject;
		FSoftClassPath TargetClassPath;
		FRuntimeGameplaySettingsPropertyEntry PropertyEntry;
		FRuntimeGameplaySettingsValue BaselineValue;
		TObjectPtr<URuntimeGameplaySettingsValueWidgetBase> ValueWidget = nullptr;
	};

	struct FRuntimeGameplaySettingsCategoryState
	{
		FString RuntimeCategoryName;
		TObjectPtr<URuntimeGameplaySettingsButtonWidget> TabButton = nullptr;
		TObjectPtr<URuntimeGameplaySettingsCategorySwitcherWidget> PageWidget = nullptr;
		TObjectPtr<UVerticalBox> ContentBox = nullptr;
		TArray<FString> TargetClassNames;
	};

	void RebuildRuntimeWidget();
	void BindControlEvents();
	void ClearGeneratedCategoryContent();
	void BuildRows();
	void AddMessageRow(UVerticalBox* ParentBox, const FText& Message);
	void AddPropertyRow(UVerticalBox* ParentBox, UObject* TargetObject, const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry);
	UVerticalBox* GetOrCreateCategoryContentBox(const FString& RuntimeCategoryName);
	FString GetRuntimeCategoryName(const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry) const;
	FRuntimeGameplaySettingsValue GetRowValue(const FRuntimeGameplaySettingsRowState& RowState) const;
	FRuntimeGameplaySettingsSnapshot BuildSnapshotFromRows() const;
	void RefreshSlotWidgets();
	const FRuntimeGameplaySettingsSlotDescriptor* FindCachedSlotByDisplayName(const FString& DisplayName) const;
	FString GetRequestedSlotName() const;
	FString GetSelectedSlotName() const;
	void SyncRequestedSlotName(const FString& SlotName);
	void UpdateSlotActionStates();
	void SetSaveSlotPanelVisible(bool bVisible);
	bool IsSaveSlotPanelVisible() const;
	void ActivateRuntimeCategory(int32 CategoryIndex);
	void ClearCategoryPages();
	int32 AddCategoryPage(UWidget* PageWidget);
	void ActivateCategoryPage(int32 CategoryIndex);
	void SetCategoryTitleFromTargetClass(const FString& RuntimeCategoryName, UClass* TargetClass);
	FString GetTargetClassDisplayName(UClass* TargetClass) const;

	UTextBlock* CreateTextBlock(const FText& Text);
	void EnsureDefaultWidgetTree();
	void BuildDefaultWidgetTree();
	FString MakeUniqueWidgetName(const FString& Prefix, const FString& SourceText, TSet<FString>& UsedNames) const;
	FString SanitizeWidgetObjectName(const FString& SourceText) const;

	UFUNCTION()
	void HandleApplyClicked();

	UFUNCTION()
	void HandleSaveClicked();

	UFUNCTION()
	void HandleSaveAsClicked();

	UFUNCTION()
	void HandleShowSaveAsClicked();

	UFUNCTION()
	void HandleCloseSaveAsClicked();

	UFUNCTION()
	void HandleLoadClicked();

	UFUNCTION()
	void HandleDeleteClicked();

	UFUNCTION()
	void HandleResetAllClicked();

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleSlotSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleSlotNameChanged(const FText& InText);

	UFUNCTION()
	void HandleCategoryTabReleased(URuntimeGameplaySettingsButtonWidget* ButtonWidget);

	UFUNCTION()
	void HandleCategorySelected(int32 CategoryIndex);

	UPROPERTY(Transient)
	TObjectPtr<URuntimeGameplaySettingsProfile> Profile;

	UPROPERTY(Transient)
	TObjectPtr<URuntimeGameplaySettingsSubsystem> SettingsSubsystem;

	TWeakObjectPtr<APlayerController> RuntimePlayerController;
	TArray<FRuntimeGameplaySettingsRowState> RowStates;
	TArray<FRuntimeGameplaySettingsCategoryState> CategoryStates;
	TSet<FString> UsedGeneratedWidgetNames;
	TArray<FRuntimeGameplaySettingsSlotDescriptor> CachedSlots;
	bool bIsSynchronizingSlots = false;
	bool bHasConstructed = false;
};
