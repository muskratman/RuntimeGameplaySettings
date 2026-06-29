#pragma once

#include "Blueprint/UserWidget.h"
#include "Data/RuntimeGameplaySettingsTypes.h"
#include "RuntimeGameplaySettingsWidget.generated.h"

class APlayerController;
class URuntimeGameplaySettingsProfile;
class URuntimeGameplaySettingsSubsystem;
class URuntimeGameplaySettingsButtonWidget;
class URuntimeGameplaySettingsButtonsOverlayWidget;
class URuntimeGameplaySettingsComponentGroupWidget;
class URuntimeGameplaySettingsCategorySwitcherWidget;
class URuntimeGameplaySettingsCategoryTabsWidget;
class URuntimeGameplaySettingsCheckboxWidget;
class URuntimeGameplaySettingsCurveAssetWidget;
class URuntimeGameplaySettingsCurveWidget;
class URuntimeGameplaySettingsEnumWidget;
class URuntimeGameplaySettingsFloatArrayWidget;
class URuntimeGameplaySettingsParameterWidget;
class URuntimeGameplaySettingsSaveSlotPanelWidget;
class URuntimeGameplaySettingsValueWidgetBase;
class URuntimeGameplaySettingsVectorWidget;
class UTextBlock;
class UEnum;
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

	virtual bool ApplyRuntimeGameplaySettingsToTargets();
	void CloseRuntimeGameplaySettingsWidget();

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(EditDefaultsOnly, Category="RuntimeGameplaySettings|UI")
	TSubclassOf<URuntimeGameplaySettingsButtonWidget> CategoryButtonWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="RuntimeGameplaySettings|UI")
	TSubclassOf<URuntimeGameplaySettingsSaveSlotPanelWidget> SaveSlotPanelWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="RuntimeGameplaySettings|UI")
	TSubclassOf<URuntimeGameplaySettingsButtonsOverlayWidget> ButtonsOverlayWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="RuntimeGameplaySettings|UI")
	TSubclassOf<URuntimeGameplaySettingsCategoryTabsWidget> CategoryTabsWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="RuntimeGameplaySettings|UI")
	TSubclassOf<URuntimeGameplaySettingsCategorySwitcherWidget> CategoryPageWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="RuntimeGameplaySettings|UI")
	TSubclassOf<URuntimeGameplaySettingsComponentGroupWidget> ComponentGroupWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="RuntimeGameplaySettings|UI|Elements")
	TSubclassOf<URuntimeGameplaySettingsCheckboxWidget> BoolPropertyWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="RuntimeGameplaySettings|UI|Elements")
	TSubclassOf<URuntimeGameplaySettingsParameterWidget> NumberPropertyWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="RuntimeGameplaySettings|UI|Elements")
	TSubclassOf<URuntimeGameplaySettingsEnumWidget> EnumPropertyWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="RuntimeGameplaySettings|UI|Elements")
	TSubclassOf<URuntimeGameplaySettingsVectorWidget> VectorPropertyWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="RuntimeGameplaySettings|UI|Elements")
	TSubclassOf<URuntimeGameplaySettingsCurveWidget> CurvePropertyWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="RuntimeGameplaySettings|UI|Elements")
	TSubclassOf<URuntimeGameplaySettingsCurveAssetWidget> CurveAssetPropertyWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="RuntimeGameplaySettings|UI|Elements")
	TSubclassOf<URuntimeGameplaySettingsFloatArrayWidget> FloatArrayPropertyWidgetClass;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<URuntimeGameplaySettingsSaveSlotPanelWidget> SaveSlotPanelWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<URuntimeGameplaySettingsButtonsOverlayWidget> ButtonsOverlayWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<URuntimeGameplaySettingsCategoryTabsWidget> CategoryTabsWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWidgetSwitcher> Switcher_DeveloperSettings;

	virtual void BuildAutomaticRows();
	virtual void BuildManualRows();

	UVerticalBox* GetOrCreateManualCategoryContentBox(const FString& RuntimeCategoryName);
	UVerticalBox* GetOrCreateManualParameterGroupContentBox(
		const FString& RuntimeCategoryName,
		const FString& GroupDisplayName);

	URuntimeGameplaySettingsCheckboxWidget* AddManualBoolParameter(
		const FString& RuntimeCategoryName,
		const FString& GroupDisplayName,
		const FText& ParameterName,
		bool bCurrentValue,
		bool bDefaultValue,
		const FText& InToolTipText = FText::GetEmpty());
	URuntimeGameplaySettingsParameterWidget* AddManualFloatParameter(
		const FString& RuntimeCategoryName,
		const FString& GroupDisplayName,
		const FText& ParameterName,
		float CurrentValue,
		float DefaultValue,
		float MinValue = -10000.0f,
		float MaxValue = 10000.0f,
		float StepSize = 0.01f,
		const FText& InToolTipText = FText::GetEmpty());
	URuntimeGameplaySettingsParameterWidget* AddManualIntParameter(
		const FString& RuntimeCategoryName,
		const FString& GroupDisplayName,
		const FText& ParameterName,
		int32 CurrentValue,
		int32 DefaultValue,
		int32 MinValue = -10000,
		int32 MaxValue = 10000,
		const FText& InToolTipText = FText::GetEmpty());
	URuntimeGameplaySettingsEnumWidget* AddManualEnumParameter(
		const FString& RuntimeCategoryName,
		const FString& GroupDisplayName,
		const FText& ParameterName,
		const UEnum* EnumClass,
		int32 CurrentValue,
		int32 DefaultValue,
		const FText& InToolTipText = FText::GetEmpty());
	URuntimeGameplaySettingsVectorWidget* AddManualVectorParameter(
		const FString& RuntimeCategoryName,
		const FString& GroupDisplayName,
		const FText& ParameterName,
		const FVector& CurrentValue,
		const FVector& DefaultValue,
		const FText& InToolTipText = FText::GetEmpty());
	URuntimeGameplaySettingsVectorWidget* AddManualRotatorParameter(
		const FString& RuntimeCategoryName,
		const FString& GroupDisplayName,
		const FText& ParameterName,
		const FRotator& CurrentValue,
		const FRotator& DefaultValue,
		const FText& InToolTipText = FText::GetEmpty());
	void AddManualParameterWidget(
		const FString& RuntimeCategoryName,
		const FString& GroupDisplayName,
		UWidget* ParameterWidget,
		const FText& InToolTipText);
	void AddManualParameterWidget(
		const FString& RuntimeCategoryName,
		const FString& GroupDisplayName,
		UWidget* ParameterWidget);
	UWidget* CreateToolTipWidget(const FText& InToolTipText);

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
		TMap<FString, TObjectPtr<URuntimeGameplaySettingsComponentGroupWidget>> ComponentGroupWidgets;
		TArray<FString> TargetClassNames;
	};

	void RebuildRuntimeWidget();
	void BindControlEvents();
	void ClearGeneratedCategoryContent();
	void BuildRows();
	void AddMessageRow(UVerticalBox* ParentBox, const FText& Message);
	void AddPropertyRow(UVerticalBox* ParentBox, UObject* TargetObject, const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry);
	UVerticalBox* GetOrCreateCategoryContentBox(const FString& RuntimeCategoryName);
	UVerticalBox* GetOrCreateComponentGroupContentBox(
		const FString& RuntimeCategoryName,
		UObject* TargetObject,
		const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry);
	UVerticalBox* GetOrCreateComponentGroupContentBox(
		const FString& RuntimeCategoryName,
		const FString& GroupDisplayName);
	FRuntimeGameplaySettingsCategoryState* FindCategoryState(const FString& RuntimeCategoryName);
	FString GetRuntimeCategoryName(const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry) const;
	FString GetComponentGroupDisplayName(UObject* TargetObject, const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry) const;
	FText GetPropertyRowLabelText(const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry) const;
	FText GetPropertyToolTipText(UObject* TargetObject, const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry) const;
	bool TryGetBaselineValue(
		UObject* TargetObject,
		const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry,
		FRuntimeGameplaySettingsValue& OutValue) const;
	bool DoesSnapshotPropertyMatchEntry(
		const FRuntimeGameplaySettingsPropertySnapshot& PropertySnapshot,
		const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry) const;
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
	void ApplyConfiguredWidgetClasses();
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
