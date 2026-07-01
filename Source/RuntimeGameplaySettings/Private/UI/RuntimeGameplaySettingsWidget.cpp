#include "UI/RuntimeGameplaySettingsWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WidgetSwitcher.h"
#include "Containers/AllowShrinking.h"
#include "Data/RuntimeGameplaySettingsProfile.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Styling/SlateBrush.h"
#include "Runtime/RuntimeGameplaySettingsPropertyAccess.h"
#include "Runtime/RuntimeGameplaySettingsSnapshotLibrary.h"
#include "Runtime/RuntimeGameplaySettingsTargetResolver.h"
#include "Settings/RuntimeGameplaySettingsProjectSettings.h"
#include "Subsystems/RuntimeGameplaySettingsSubsystem.h"
#include "Templates/UnrealTemplate.h"
#include "UI/Elements/RuntimeGameplaySettingsButtonWidget.h"
#include "UI/Elements/RuntimeGameplaySettingsCheckboxWidget.h"
#include "UI/Elements/RuntimeGameplaySettingsCurveAssetWidget.h"
#include "UI/Elements/RuntimeGameplaySettingsCurveWidget.h"
#include "UI/Elements/RuntimeGameplaySettingsEnumWidget.h"
#include "UI/Elements/RuntimeGameplaySettingsElementWidgetHelpers.h"
#include "UI/Elements/RuntimeGameplaySettingsFloatArrayWidget.h"
#include "UI/Elements/RuntimeGameplaySettingsParameterWidget.h"
#include "UI/Elements/RuntimeGameplaySettingsValueWidgetBase.h"
#include "UI/Elements/RuntimeGameplaySettingsVectorWidget.h"
#include "UI/RuntimeGameplaySettingsButtonsOverlayWidget.h"
#include "UI/RuntimeGameplaySettingsCategorySwitcherWidget.h"
#include "UI/RuntimeGameplaySettingsCategoryTabsWidget.h"
#include "UI/RuntimeGameplaySettingsComponentGroupWidget.h"
#include "UI/RuntimeGameplaySettingsSaveSlotPanelWidget.h"

namespace
{
	constexpr int32 RuntimeGameplaySettingsBaseToolTipFontSize = 10;
	constexpr float RuntimeGameplaySettingsToolTipFontScale = 1.3f;

	template <typename WidgetType>
	TSubclassOf<WidgetType> ResolveConfiguredWidgetClass(
		const TSoftClassPtr<WidgetType>& ConfiguredClass,
		TSubclassOf<WidgetType> FallbackClass)
	{
		if (UClass* LoadedClass = ConfiguredClass.LoadSynchronous())
		{
			if (LoadedClass->IsChildOf(WidgetType::StaticClass()))
			{
				return LoadedClass;
			}
		}

		if (FallbackClass)
		{
			return FallbackClass;
		}

		return WidgetType::StaticClass();
	}
}

URuntimeGameplaySettingsWidget::URuntimeGameplaySettingsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CategoryButtonWidgetClass = URuntimeGameplaySettingsButtonWidget::StaticClass();
	SaveSlotPanelWidgetClass = URuntimeGameplaySettingsSaveSlotPanelWidget::StaticClass();
	ButtonsOverlayWidgetClass = URuntimeGameplaySettingsButtonsOverlayWidget::StaticClass();
	CategoryTabsWidgetClass = URuntimeGameplaySettingsCategoryTabsWidget::StaticClass();
	CategoryPageWidgetClass = URuntimeGameplaySettingsCategorySwitcherWidget::StaticClass();
	ComponentGroupWidgetClass = URuntimeGameplaySettingsComponentGroupWidget::StaticClass();
	BoolPropertyWidgetClass = URuntimeGameplaySettingsCheckboxWidget::StaticClass();
	NumberPropertyWidgetClass = URuntimeGameplaySettingsParameterWidget::StaticClass();
	EnumPropertyWidgetClass = URuntimeGameplaySettingsEnumWidget::StaticClass();
	VectorPropertyWidgetClass = URuntimeGameplaySettingsVectorWidget::StaticClass();
	CurvePropertyWidgetClass = URuntimeGameplaySettingsCurveWidget::StaticClass();
	CurveAssetPropertyWidgetClass = URuntimeGameplaySettingsCurveAssetWidget::StaticClass();
	FloatArrayPropertyWidgetClass = URuntimeGameplaySettingsFloatArrayWidget::StaticClass();
}

void URuntimeGameplaySettingsWidget::InitializeRuntimeGameplaySettings(
	APlayerController* InPlayerController,
	URuntimeGameplaySettingsProfile* InProfile,
	URuntimeGameplaySettingsSubsystem* InSubsystem)
{
	RuntimePlayerController = InPlayerController;
	Profile = InProfile;
	SettingsSubsystem = InSubsystem;

	if (bHasConstructed)
	{
		RebuildRuntimeWidget();
	}
}

void URuntimeGameplaySettingsWidget::CloseRuntimeGameplaySettingsWidget()
{
	ApplyRuntimeGameplaySettingsToTargets();
	OnRequestClose.ExecuteIfBound();
}

bool URuntimeGameplaySettingsWidget::ApplyRuntimeGameplaySettingsToTargets()
{
	APlayerController* PlayerController = RuntimePlayerController.Get();
	if (!PlayerController || !Profile)
	{
		return false;
	}

	FRuntimeGameplaySettingsSnapshotLibrary::ApplySnapshot(
		PlayerController,
		Profile,
		BuildSnapshotFromRows());
	return true;
}

void URuntimeGameplaySettingsWidget::NativeOnInitialized()
{
	EnsureDefaultWidgetTree();
	Super::NativeOnInitialized();
	BindControlEvents();
}

void URuntimeGameplaySettingsWidget::NativePreConstruct()
{
	EnsureDefaultWidgetTree();
	Super::NativePreConstruct();
}

void URuntimeGameplaySettingsWidget::NativeConstruct()
{
	EnsureDefaultWidgetTree();
	Super::NativeConstruct();
	bHasConstructed = true;
	RebuildRuntimeWidget();
}

FReply URuntimeGameplaySettingsWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	(void)InGeometry;

	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		CloseRuntimeGameplaySettingsWidget();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void URuntimeGameplaySettingsWidget::RebuildRuntimeWidget()
{
	EnsureDefaultWidgetTree();
	ApplyConfiguredWidgetClasses();

	if (!WidgetTree)
	{
		return;
	}

	RowStates.Reset();
	UsedGeneratedWidgetNames.Reset();

	BindControlEvents();
	SetSaveSlotPanelVisible(false);
	ClearGeneratedCategoryContent();
	BuildRows();
	RefreshSlotWidgets();
}

void URuntimeGameplaySettingsWidget::BindControlEvents()
{
	if (SaveSlotPanelWidget)
	{
		SaveSlotPanelWidget->OnCloseRequested.AddUniqueDynamic(
			this,
			&URuntimeGameplaySettingsWidget::HandleCloseSaveAsClicked);
		SaveSlotPanelWidget->OnSaveAsRequested.AddUniqueDynamic(this, &URuntimeGameplaySettingsWidget::HandleSaveAsClicked);
		SaveSlotPanelWidget->OnLoadRequested.AddUniqueDynamic(this, &URuntimeGameplaySettingsWidget::HandleLoadClicked);
		SaveSlotPanelWidget->OnDeleteRequested.AddUniqueDynamic(this, &URuntimeGameplaySettingsWidget::HandleDeleteClicked);
		SaveSlotPanelWidget->OnSlotSelectionChanged.AddUniqueDynamic(
			this,
			&URuntimeGameplaySettingsWidget::HandleSlotSelectionChanged);
		SaveSlotPanelWidget->OnSlotNameChanged.AddUniqueDynamic(
			this,
			&URuntimeGameplaySettingsWidget::HandleSlotNameChanged);
	}

	if (ButtonsOverlayWidget)
	{
		ButtonsOverlayWidget->OnShowSaveAsRequested.AddUniqueDynamic(
			this,
			&URuntimeGameplaySettingsWidget::HandleShowSaveAsClicked);
		ButtonsOverlayWidget->OnSaveRequested.AddUniqueDynamic(this, &URuntimeGameplaySettingsWidget::HandleSaveClicked);
		ButtonsOverlayWidget->OnCloseRequested.AddUniqueDynamic(this, &URuntimeGameplaySettingsWidget::HandleCloseClicked);
		ButtonsOverlayWidget->OnResetAllToDefaultRequested.AddUniqueDynamic(
			this,
			&URuntimeGameplaySettingsWidget::HandleResetAllClicked);
	}

	if (CategoryTabsWidget)
	{
		CategoryTabsWidget->OnCategorySelected.AddUniqueDynamic(
			this,
			&URuntimeGameplaySettingsWidget::HandleCategorySelected);
	}
}

void URuntimeGameplaySettingsWidget::ClearGeneratedCategoryContent()
{
	CategoryStates.Reset();

	if (CategoryTabsWidget)
	{
		CategoryTabsWidget->ClearCategories();
	}

	ClearCategoryPages();
}

void URuntimeGameplaySettingsWidget::BuildRows()
{
	BuildAutomaticRows();
	BuildManualRows();
	ActivateRuntimeCategory(0);
}

void URuntimeGameplaySettingsWidget::BuildAutomaticRows()
{
	UVerticalBox* CommonCategoryBox = GetOrCreateCategoryContentBox(TEXT("Common"));

	if (!Profile)
	{
		AddMessageRow(CommonCategoryBox, INVTEXT("No RuntimeGameplaySettings profile configured."));
		return;
	}

	APlayerController* PlayerController = RuntimePlayerController.Get();
	if (!PlayerController)
	{
		AddMessageRow(CommonCategoryBox, INVTEXT("No local PlayerController found."));
		return;
	}

	if (SettingsSubsystem && !SettingsSubsystem->HasBaselineSnapshot())
	{
		SettingsSubsystem->SetBaselineSnapshot(
			FRuntimeGameplaySettingsSnapshotLibrary::CaptureSnapshot(PlayerController, Profile));
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

		UObject* TargetObject = Targets[0];
		for (const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry : ClassEntry.Properties)
		{
			const FString RuntimeCategoryName = GetRuntimeCategoryName(PropertyEntry);
			UVerticalBox* PropertyParentBox =
				GetOrCreateComponentGroupContentBox(RuntimeCategoryName, TargetObject, PropertyEntry);
			if (!PropertyParentBox)
			{
				continue;
			}

			SetCategoryTitleFromTargetClass(RuntimeCategoryName, TargetClass);

			AddPropertyRow(PropertyParentBox, TargetObject, PropertyEntry);
		}
	}

	if (RowStates.Num() == 0)
	{
		AddMessageRow(
			CommonCategoryBox,
			INVTEXT("No matching runtime targets or supported properties found."));
	}

}

void URuntimeGameplaySettingsWidget::BuildManualRows()
{
}

void URuntimeGameplaySettingsWidget::AddMessageRow(UVerticalBox* ParentBox, const FText& Message)
{
	if (ParentBox)
	{
		ParentBox->AddChildToVerticalBox(CreateTextBlock(Message));
	}
}

void URuntimeGameplaySettingsWidget::AddPropertyRow(
	UVerticalBox* ParentBox,
	UObject* TargetObject,
	const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry)
{
	if (!ParentBox || !TargetObject)
	{
		return;
	}

	FRuntimeGameplaySettingsValue CurrentValue;
	if (!FRuntimeGameplaySettingsPropertyAccess::ReadValue(TargetObject, PropertyEntry, CurrentValue))
	{
		return;
	}

	FRuntimeGameplaySettingsValue BaselineValue = CurrentValue;
	TryGetBaselineValue(TargetObject, PropertyEntry, BaselineValue);

	const FText LabelText = GetPropertyRowLabelText(PropertyEntry);

	const FString WidgetName = MakeUniqueWidgetName(TEXT("Property"), LabelText.ToString(), UsedGeneratedWidgetNames);
	URuntimeGameplaySettingsValueWidgetBase* ValueWidget = nullptr;

	FRuntimeGameplaySettingsRowState RowState;
	RowState.TargetObject = TargetObject;
	RowState.TargetClassPath = FSoftClassPath(TargetObject->GetClass());
	RowState.PropertyEntry = PropertyEntry;
	RowState.BaselineValue = BaselineValue;

	switch (PropertyEntry.ValueType)
	{
	case ERuntimeGameplaySettingsValueType::Bool:
		if (BoolPropertyWidgetClass)
		{
			if (URuntimeGameplaySettingsCheckboxWidget* CheckboxWidget =
			WidgetTree->ConstructWidget<URuntimeGameplaySettingsCheckboxWidget>(
				BoolPropertyWidgetClass,
				FName(*WidgetName)))
			{
				CheckboxWidget->Initialize();
				CheckboxWidget->SetParameterName(LabelText);
				CheckboxWidget->SetCheckboxValue(CurrentValue.BoolValue);
				CheckboxWidget->SetDefaultCheckboxValue(BaselineValue.BoolValue);
				ValueWidget = CheckboxWidget;
			}
		}
		break;
	case ERuntimeGameplaySettingsValueType::Float:
		if (NumberPropertyWidgetClass)
		{
			if (URuntimeGameplaySettingsParameterWidget* ParameterWidget =
			WidgetTree->ConstructWidget<URuntimeGameplaySettingsParameterWidget>(
				NumberPropertyWidgetClass,
				FName(*WidgetName)))
			{
				ParameterWidget->Initialize();
				ParameterWidget->SetParameterName(LabelText);
				ParameterWidget->SetParameterValue(CurrentValue.FloatValue);
				ParameterWidget->SetDefaultParameterValue(BaselineValue.FloatValue);
				ValueWidget = ParameterWidget;
			}
		}
		break;
	case ERuntimeGameplaySettingsValueType::Int:
		if (NumberPropertyWidgetClass)
		{
			if (URuntimeGameplaySettingsParameterWidget* ParameterWidget =
			WidgetTree->ConstructWidget<URuntimeGameplaySettingsParameterWidget>(
				NumberPropertyWidgetClass,
				FName(*WidgetName)))
			{
				ParameterWidget->Initialize();
				ParameterWidget->SetParameterName(LabelText);
				ParameterWidget->SetParameterValue(static_cast<float>(CurrentValue.IntValue));
				ParameterWidget->SetDefaultParameterValue(static_cast<float>(BaselineValue.IntValue));
				ValueWidget = ParameterWidget;
			}
		}
		break;
	case ERuntimeGameplaySettingsValueType::Enum:
		if (EnumPropertyWidgetClass)
		{
			if (URuntimeGameplaySettingsEnumWidget* EnumWidget =
			WidgetTree->ConstructWidget<URuntimeGameplaySettingsEnumWidget>(
				EnumPropertyWidgetClass,
				FName(*WidgetName)))
			{
				EnumWidget->Initialize();
				const UEnum* RuntimeEnum = PropertyEntry.EnumPath.IsEmpty()
					? nullptr
					: LoadObject<UEnum>(nullptr, *PropertyEntry.EnumPath);
				EnumWidget->SetParameterName(LabelText);
				EnumWidget->SetOptionsFromEnum(RuntimeEnum);
				EnumWidget->SetSelectedValue(CurrentValue.IntValue);
				EnumWidget->SetDefaultSelectedValue(BaselineValue.IntValue);
				ValueWidget = EnumWidget;
			}
		}
		break;
	case ERuntimeGameplaySettingsValueType::Vector:
		if (VectorPropertyWidgetClass)
		{
			if (URuntimeGameplaySettingsVectorWidget* VectorWidget =
			WidgetTree->ConstructWidget<URuntimeGameplaySettingsVectorWidget>(
				VectorPropertyWidgetClass,
				FName(*WidgetName)))
			{
				VectorWidget->Initialize();
				VectorWidget->SetParameterName(LabelText);
				VectorWidget->SetVectorValue(CurrentValue.VectorValue);
				VectorWidget->SetDefaultVectorValue(BaselineValue.VectorValue);
				ValueWidget = VectorWidget;
			}
		}
		break;
	case ERuntimeGameplaySettingsValueType::Rotator:
		if (VectorPropertyWidgetClass)
		{
			if (URuntimeGameplaySettingsVectorWidget* VectorWidget =
			WidgetTree->ConstructWidget<URuntimeGameplaySettingsVectorWidget>(
				VectorPropertyWidgetClass,
				FName(*WidgetName)))
			{
				VectorWidget->Initialize();
				const FVector RotatorAsVector(
					CurrentValue.RotatorValue.Pitch,
					CurrentValue.RotatorValue.Yaw,
					CurrentValue.RotatorValue.Roll);
				const FVector BaselineRotatorAsVector(
					BaselineValue.RotatorValue.Pitch,
					BaselineValue.RotatorValue.Yaw,
					BaselineValue.RotatorValue.Roll);
				VectorWidget->SetParameterName(LabelText);
				VectorWidget->SetVectorValue(RotatorAsVector);
				VectorWidget->SetDefaultVectorValue(BaselineRotatorAsVector);
				ValueWidget = VectorWidget;
			}
		}
		break;
	default:
		return;
	}

	if (!ValueWidget)
	{
		return;
	}

	RowState.ValueWidget = ValueWidget;
	const FText PropertyToolTipText = GetPropertyToolTipText(TargetObject, PropertyEntry);
	if (!PropertyToolTipText.IsEmpty())
	{
		ValueWidget->SetToolTip(CreateToolTipWidget(PropertyToolTipText));
	}
	if (UVerticalBoxSlot* RowSlot = ParentBox->AddChildToVerticalBox(ValueWidget))
	{
		RowSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
	}

	RowStates.Add(RowState);
}

UVerticalBox* URuntimeGameplaySettingsWidget::GetOrCreateCategoryContentBox(const FString& RuntimeCategoryName)
{
	for (FRuntimeGameplaySettingsCategoryState& CategoryState : CategoryStates)
	{
		if (CategoryState.RuntimeCategoryName.Equals(RuntimeCategoryName, ESearchCase::IgnoreCase))
		{
			return CategoryState.ContentBox;
		}
	}

	if (!WidgetTree || !CategoryTabsWidget || !Switcher_DeveloperSettings || !CategoryPageWidgetClass)
	{
		return nullptr;
	}

	FRuntimeGameplaySettingsCategoryState NewState;
	NewState.RuntimeCategoryName = RuntimeCategoryName;

	const FString TabName = MakeUniqueWidgetName(TEXT("ButtTab"), RuntimeCategoryName, UsedGeneratedWidgetNames);
	if (CategoryTabsWidget)
	{
		NewState.TabButton = CategoryTabsWidget->AddCategory(
			FText::FromString(RuntimeCategoryName),
			FName(*TabName),
			CategoryButtonWidgetClass);
	}

	TSubclassOf<URuntimeGameplaySettingsCategorySwitcherWidget> ResolvedPageClass = CategoryPageWidgetClass;
	const FString PageName = MakeUniqueWidgetName(TEXT("CategoryPage"), RuntimeCategoryName, UsedGeneratedWidgetNames);
	NewState.PageWidget = WidgetTree->ConstructWidget<URuntimeGameplaySettingsCategorySwitcherWidget>(
		ResolvedPageClass,
		FName(*PageName));
	if (!NewState.PageWidget)
	{
		return nullptr;
	}

	NewState.PageWidget->Initialize();
	NewState.PageWidget->SetTitleText(FText::FromString(RuntimeCategoryName));
	NewState.ContentBox = NewState.PageWidget->GetOrCreatePropertiesBox();
	if (!NewState.ContentBox)
	{
		return nullptr;
	}

	AddCategoryPage(NewState.PageWidget);

	CategoryStates.Add(NewState);
	return CategoryStates.Last().ContentBox;
}

UVerticalBox* URuntimeGameplaySettingsWidget::GetOrCreateComponentGroupContentBox(
	const FString& RuntimeCategoryName,
	UObject* TargetObject,
	const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry)
{
	const FString ComponentGroupName = GetComponentGroupDisplayName(TargetObject, PropertyEntry);
	return GetOrCreateComponentGroupContentBox(RuntimeCategoryName, ComponentGroupName);
}

UVerticalBox* URuntimeGameplaySettingsWidget::GetOrCreateComponentGroupContentBox(
	const FString& RuntimeCategoryName,
	const FString& GroupDisplayName)
{
	UVerticalBox* CategoryBox = GetOrCreateCategoryContentBox(RuntimeCategoryName);
	if (!CategoryBox || !WidgetTree || !ComponentGroupWidgetClass)
	{
		return CategoryBox;
	}

	FRuntimeGameplaySettingsCategoryState* CategoryState = FindCategoryState(RuntimeCategoryName);
	if (!CategoryState)
	{
		return CategoryBox;
	}

	const FString ComponentGroupName = GroupDisplayName.TrimStartAndEnd();
	if (ComponentGroupName.IsEmpty())
	{
		return CategoryBox;
	}

	if (TObjectPtr<URuntimeGameplaySettingsComponentGroupWidget>* ExistingGroup =
		CategoryState->ComponentGroupWidgets.Find(ComponentGroupName))
	{
		return ExistingGroup->Get()
			? ExistingGroup->Get()->GetOrCreatePropertiesBox()
			: CategoryBox;
	}

	const FString GroupWidgetName = MakeUniqueWidgetName(
		TEXT("ComponentGroup"),
		RuntimeCategoryName + TEXT("_") + ComponentGroupName,
		UsedGeneratedWidgetNames);
	URuntimeGameplaySettingsComponentGroupWidget* ComponentGroupWidget =
		WidgetTree->ConstructWidget<URuntimeGameplaySettingsComponentGroupWidget>(
			ComponentGroupWidgetClass,
			FName(*GroupWidgetName));
	if (!ComponentGroupWidget)
	{
		return CategoryBox;
	}

	ComponentGroupWidget->Initialize();
	ComponentGroupWidget->SetComponentTitleText(FText::FromString(ComponentGroupName));
	if (UVerticalBoxSlot* GroupSlot = CategoryBox->AddChildToVerticalBox(ComponentGroupWidget))
	{
		GroupSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 20.0f));
		GroupSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	CategoryState->ComponentGroupWidgets.Add(ComponentGroupName, ComponentGroupWidget);
	if (UVerticalBox* GroupPropertiesBox = ComponentGroupWidget->GetOrCreatePropertiesBox())
	{
		return GroupPropertiesBox;
	}

	return CategoryBox;
}

UVerticalBox* URuntimeGameplaySettingsWidget::GetOrCreateManualCategoryContentBox(
	const FString& RuntimeCategoryName)
{
	FString SanitizedCategoryName = RuntimeCategoryName.TrimStartAndEnd();
	if (SanitizedCategoryName.IsEmpty() || SanitizedCategoryName.Equals(TEXT("Common"), ESearchCase::IgnoreCase))
	{
		SanitizedCategoryName = TEXT("Common");
	}

	return GetOrCreateCategoryContentBox(SanitizedCategoryName);
}

UVerticalBox* URuntimeGameplaySettingsWidget::GetOrCreateManualParameterGroupContentBox(
	const FString& RuntimeCategoryName,
	const FString& GroupDisplayName)
{
	const FString SanitizedCategoryName = RuntimeCategoryName.TrimStartAndEnd().IsEmpty()
		? FString(TEXT("Common"))
		: RuntimeCategoryName.TrimStartAndEnd();
	return GetOrCreateComponentGroupContentBox(SanitizedCategoryName, GroupDisplayName);
}

URuntimeGameplaySettingsCheckboxWidget* URuntimeGameplaySettingsWidget::AddManualBoolParameter(
	const FString& RuntimeCategoryName,
	const FString& GroupDisplayName,
	const FText& ParameterName,
	bool bCurrentValue,
	bool bDefaultValue,
	const FText& InToolTipText)
{
	if (!WidgetTree || !BoolPropertyWidgetClass)
	{
		return nullptr;
	}

	const FString WidgetName = MakeUniqueWidgetName(TEXT("ManualBool"), ParameterName.ToString(), UsedGeneratedWidgetNames);
	URuntimeGameplaySettingsCheckboxWidget* CheckboxWidget =
		WidgetTree->ConstructWidget<URuntimeGameplaySettingsCheckboxWidget>(
			BoolPropertyWidgetClass,
			FName(*WidgetName));
	if (!CheckboxWidget)
	{
		return nullptr;
	}

	CheckboxWidget->Initialize();
	CheckboxWidget->SetParameterName(ParameterName);
	CheckboxWidget->SetCheckboxValue(bCurrentValue);
	CheckboxWidget->SetDefaultCheckboxValue(bDefaultValue);
	AddManualParameterWidget(RuntimeCategoryName, GroupDisplayName, CheckboxWidget, InToolTipText);
	return CheckboxWidget;
}

URuntimeGameplaySettingsParameterWidget* URuntimeGameplaySettingsWidget::AddManualFloatParameter(
	const FString& RuntimeCategoryName,
	const FString& GroupDisplayName,
	const FText& ParameterName,
	float CurrentValue,
	float DefaultValue,
	float MinValue,
	float MaxValue,
	float StepSize,
	const FText& InToolTipText)
{
	if (!WidgetTree || !NumberPropertyWidgetClass)
	{
		return nullptr;
	}

	const FString WidgetName = MakeUniqueWidgetName(TEXT("ManualFloat"), ParameterName.ToString(), UsedGeneratedWidgetNames);
	URuntimeGameplaySettingsParameterWidget* ParameterWidget =
		WidgetTree->ConstructWidget<URuntimeGameplaySettingsParameterWidget>(
			NumberPropertyWidgetClass,
			FName(*WidgetName));
	if (!ParameterWidget)
	{
		return nullptr;
	}

	ParameterWidget->Initialize();
	ParameterWidget->SetParameterName(ParameterName);
	ParameterWidget->SetParameterRange(MinValue, MaxValue, StepSize);
	ParameterWidget->SetParameterValue(CurrentValue);
	ParameterWidget->SetDefaultParameterValue(DefaultValue);
	AddManualParameterWidget(RuntimeCategoryName, GroupDisplayName, ParameterWidget, InToolTipText);
	return ParameterWidget;
}

URuntimeGameplaySettingsParameterWidget* URuntimeGameplaySettingsWidget::AddManualIntParameter(
	const FString& RuntimeCategoryName,
	const FString& GroupDisplayName,
	const FText& ParameterName,
	int32 CurrentValue,
	int32 DefaultValue,
	int32 MinValue,
	int32 MaxValue,
	const FText& InToolTipText)
{
	return AddManualFloatParameter(
		RuntimeCategoryName,
		GroupDisplayName,
		ParameterName,
		static_cast<float>(CurrentValue),
		static_cast<float>(DefaultValue),
		static_cast<float>(MinValue),
		static_cast<float>(MaxValue),
		1.0f,
		InToolTipText);
}

URuntimeGameplaySettingsEnumWidget* URuntimeGameplaySettingsWidget::AddManualEnumParameter(
	const FString& RuntimeCategoryName,
	const FString& GroupDisplayName,
	const FText& ParameterName,
	const UEnum* EnumClass,
	int32 CurrentValue,
	int32 DefaultValue,
	const FText& InToolTipText)
{
	if (!WidgetTree || !EnumPropertyWidgetClass)
	{
		return nullptr;
	}

	const FString WidgetName = MakeUniqueWidgetName(TEXT("ManualEnum"), ParameterName.ToString(), UsedGeneratedWidgetNames);
	URuntimeGameplaySettingsEnumWidget* EnumWidget =
		WidgetTree->ConstructWidget<URuntimeGameplaySettingsEnumWidget>(
			EnumPropertyWidgetClass,
			FName(*WidgetName));
	if (!EnumWidget)
	{
		return nullptr;
	}

	EnumWidget->Initialize();
	EnumWidget->SetParameterName(ParameterName);
	EnumWidget->SetOptionsFromEnum(EnumClass);
	EnumWidget->SetSelectedValue(CurrentValue);
	EnumWidget->SetDefaultSelectedValue(DefaultValue);
	AddManualParameterWidget(RuntimeCategoryName, GroupDisplayName, EnumWidget, InToolTipText);
	return EnumWidget;
}

URuntimeGameplaySettingsVectorWidget* URuntimeGameplaySettingsWidget::AddManualVectorParameter(
	const FString& RuntimeCategoryName,
	const FString& GroupDisplayName,
	const FText& ParameterName,
	const FVector& CurrentValue,
	const FVector& DefaultValue,
	const FText& InToolTipText)
{
	if (!WidgetTree || !VectorPropertyWidgetClass)
	{
		return nullptr;
	}

	const FString WidgetName = MakeUniqueWidgetName(TEXT("ManualVector"), ParameterName.ToString(), UsedGeneratedWidgetNames);
	URuntimeGameplaySettingsVectorWidget* VectorWidget =
		WidgetTree->ConstructWidget<URuntimeGameplaySettingsVectorWidget>(
			VectorPropertyWidgetClass,
			FName(*WidgetName));
	if (!VectorWidget)
	{
		return nullptr;
	}

	VectorWidget->Initialize();
	VectorWidget->SetParameterName(ParameterName);
	VectorWidget->SetVectorValue(CurrentValue);
	VectorWidget->SetDefaultVectorValue(DefaultValue);
	AddManualParameterWidget(RuntimeCategoryName, GroupDisplayName, VectorWidget, InToolTipText);
	return VectorWidget;
}

URuntimeGameplaySettingsVectorWidget* URuntimeGameplaySettingsWidget::AddManualRotatorParameter(
	const FString& RuntimeCategoryName,
	const FString& GroupDisplayName,
	const FText& ParameterName,
	const FRotator& CurrentValue,
	const FRotator& DefaultValue,
	const FText& InToolTipText)
{
	return AddManualVectorParameter(
		RuntimeCategoryName,
		GroupDisplayName,
		ParameterName,
		FVector(CurrentValue.Pitch, CurrentValue.Yaw, CurrentValue.Roll),
		FVector(DefaultValue.Pitch, DefaultValue.Yaw, DefaultValue.Roll),
		InToolTipText);
}

void URuntimeGameplaySettingsWidget::AddManualParameterWidget(
	const FString& RuntimeCategoryName,
	const FString& GroupDisplayName,
	UWidget* ParameterWidget,
	const FText& InToolTipText)
{
	if (!ParameterWidget)
	{
		return;
	}

	UVerticalBox* ParentBox = GroupDisplayName.TrimStartAndEnd().IsEmpty()
		? GetOrCreateManualCategoryContentBox(RuntimeCategoryName)
		: GetOrCreateManualParameterGroupContentBox(RuntimeCategoryName, GroupDisplayName);
	if (!ParentBox)
	{
		return;
	}

	if (!InToolTipText.IsEmpty())
	{
		ParameterWidget->SetToolTip(CreateToolTipWidget(InToolTipText));
	}

	if (UVerticalBoxSlot* ParameterSlot = ParentBox->AddChildToVerticalBox(ParameterWidget))
	{
		ParameterSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
	}
}

void URuntimeGameplaySettingsWidget::AddManualParameterWidget(
	const FString& RuntimeCategoryName,
	const FString& GroupDisplayName,
	UWidget* ParameterWidget)
{
	AddManualParameterWidget(RuntimeCategoryName, GroupDisplayName, ParameterWidget, FText::GetEmpty());
}

URuntimeGameplaySettingsWidget::FRuntimeGameplaySettingsCategoryState*
URuntimeGameplaySettingsWidget::FindCategoryState(const FString& RuntimeCategoryName)
{
	for (FRuntimeGameplaySettingsCategoryState& CategoryState : CategoryStates)
	{
		if (CategoryState.RuntimeCategoryName.Equals(RuntimeCategoryName, ESearchCase::IgnoreCase))
		{
			return &CategoryState;
		}
	}

	return nullptr;
}

FString URuntimeGameplaySettingsWidget::GetRuntimeCategoryName(
	const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry) const
{
	FString RuntimeCategoryName = PropertyEntry.RuntimeCategory.TrimStartAndEnd();
	if (RuntimeCategoryName.IsEmpty() || RuntimeCategoryName.Equals(TEXT("Common"), ESearchCase::IgnoreCase))
	{
		RuntimeCategoryName = TEXT("Common");
	}
	return RuntimeCategoryName;
}

FString URuntimeGameplaySettingsWidget::GetComponentGroupDisplayName(
	UObject* TargetObject,
	const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry) const
{
	if (!PropertyEntry.ComponentName.IsNone())
	{
		return PropertyEntry.ComponentName.ToString();
	}

	if (TargetObject && TargetObject->GetClass())
	{
		return GetTargetClassDisplayName(TargetObject->GetClass());
	}

	return TEXT("Object");
}

FText URuntimeGameplaySettingsWidget::GetPropertyRowLabelText(
	const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry) const
{
	FString LabelString = PropertyEntry.DisplayName.IsEmpty()
		? FRuntimeGameplaySettingsPropertyAccess::BuildPropertyPathString(
			PropertyEntry.PropertyPath,
			PropertyEntry.PropertyName)
		: PropertyEntry.DisplayName.ToString();

	if (!PropertyEntry.ComponentName.IsNone())
	{
		const FString ComponentPrefix = PropertyEntry.ComponentName.ToString() + TEXT(".");
		LabelString.RemoveFromStart(ComponentPrefix, ESearchCase::IgnoreCase);
	}

	if (LabelString.IsEmpty())
	{
		LabelString = PropertyEntry.PropertyName.ToString();
	}

	return FText::FromString(LabelString);
}

FText URuntimeGameplaySettingsWidget::GetPropertyToolTipText(
	UObject* TargetObject,
	const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry) const
{
	const FProperty* RuntimeProperty =
		FRuntimeGameplaySettingsPropertyAccess::FindRuntimeProperty(TargetObject, PropertyEntry);
	if (!RuntimeProperty)
	{
		return FText::GetEmpty();
	}

	const FText PropertyToolTipText = RuntimeProperty->GetToolTipText();
	if (!PropertyToolTipText.IsEmpty())
	{
		return PropertyToolTipText;
	}

	const FString ToolTipMetadata = RuntimeProperty->GetMetaData(TEXT("ToolTip"));
	return ToolTipMetadata.IsEmpty() ? FText::GetEmpty() : FText::FromString(ToolTipMetadata);
}

bool URuntimeGameplaySettingsWidget::TryGetBaselineValue(
	UObject* TargetObject,
	const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry,
	FRuntimeGameplaySettingsValue& OutValue) const
{
	if (!TargetObject || !SettingsSubsystem)
	{
		return false;
	}

	FRuntimeGameplaySettingsSnapshot BaselineSnapshot;
	if (!SettingsSubsystem->TryGetBaselineSnapshot(BaselineSnapshot))
	{
		return false;
	}

	for (const FRuntimeGameplaySettingsObjectSnapshot& ObjectSnapshot : BaselineSnapshot.Objects)
	{
		UClass* SnapshotTargetClass = ObjectSnapshot.TargetClassPath.TryLoadClass<UObject>();
		if (!SnapshotTargetClass || !TargetObject->IsA(SnapshotTargetClass))
		{
			continue;
		}

		for (const FRuntimeGameplaySettingsPropertySnapshot& PropertySnapshot : ObjectSnapshot.Properties)
		{
			if (DoesSnapshotPropertyMatchEntry(PropertySnapshot, PropertyEntry))
			{
				OutValue = PropertySnapshot.Value;
				return OutValue.ValueType == PropertyEntry.ValueType;
			}
		}
	}

	return false;
}

bool URuntimeGameplaySettingsWidget::DoesSnapshotPropertyMatchEntry(
	const FRuntimeGameplaySettingsPropertySnapshot& PropertySnapshot,
	const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry) const
{
	return PropertySnapshot.bIsComponentProperty == PropertyEntry.bIsComponentProperty
		&& PropertySnapshot.ComponentName == PropertyEntry.ComponentName
		&& PropertySnapshot.ComponentClassPath.Equals(PropertyEntry.ComponentClassPath, ESearchCase::IgnoreCase)
		&& PropertySnapshot.PropertyName == PropertyEntry.PropertyName
		&& PropertySnapshot.PropertyPath == PropertyEntry.PropertyPath
		&& PropertySnapshot.ValueType == PropertyEntry.ValueType
		&& PropertySnapshot.EnumPath.Equals(PropertyEntry.EnumPath, ESearchCase::IgnoreCase);
}

FRuntimeGameplaySettingsValue URuntimeGameplaySettingsWidget::GetRowValue(
	const FRuntimeGameplaySettingsRowState& RowState) const
{
	FRuntimeGameplaySettingsValue Value;
	Value.ValueType = RowState.PropertyEntry.ValueType;

	switch (RowState.PropertyEntry.ValueType)
	{
	case ERuntimeGameplaySettingsValueType::Bool:
		if (const URuntimeGameplaySettingsCheckboxWidget* CheckboxWidget =
			Cast<URuntimeGameplaySettingsCheckboxWidget>(RowState.ValueWidget))
		{
			Value.BoolValue = CheckboxWidget->GetCheckboxValue();
		}
		break;
	case ERuntimeGameplaySettingsValueType::Float:
		Value.FloatValue = RowState.BaselineValue.FloatValue;
		if (const URuntimeGameplaySettingsParameterWidget* ParameterWidget =
			Cast<URuntimeGameplaySettingsParameterWidget>(RowState.ValueWidget))
		{
			Value.FloatValue = ParameterWidget->GetEditableParameterValue();
		}
		break;
	case ERuntimeGameplaySettingsValueType::Int:
		Value.IntValue = RowState.BaselineValue.IntValue;
		if (const URuntimeGameplaySettingsParameterWidget* ParameterWidget =
			Cast<URuntimeGameplaySettingsParameterWidget>(RowState.ValueWidget))
		{
			Value.IntValue = FMath::RoundToInt(ParameterWidget->GetEditableParameterValue());
		}
		break;
	case ERuntimeGameplaySettingsValueType::Enum:
		Value.IntValue = RowState.BaselineValue.IntValue;
		if (const URuntimeGameplaySettingsEnumWidget* EnumWidget =
			Cast<URuntimeGameplaySettingsEnumWidget>(RowState.ValueWidget))
		{
			Value.IntValue = EnumWidget->GetSelectedValue();
		}
		break;
	case ERuntimeGameplaySettingsValueType::Vector:
		if (const URuntimeGameplaySettingsVectorWidget* VectorWidget =
			Cast<URuntimeGameplaySettingsVectorWidget>(RowState.ValueWidget))
		{
			Value.VectorValue = VectorWidget->GetVectorValue();
		}
		else
		{
			Value.VectorValue = RowState.BaselineValue.VectorValue;
		}
		break;
	case ERuntimeGameplaySettingsValueType::Rotator:
		if (const URuntimeGameplaySettingsVectorWidget* VectorWidget =
			Cast<URuntimeGameplaySettingsVectorWidget>(RowState.ValueWidget))
		{
			const FVector RotatorAsVector = VectorWidget->GetVectorValue();
			Value.RotatorValue = FRotator(RotatorAsVector.X, RotatorAsVector.Y, RotatorAsVector.Z);
		}
		else
		{
			Value.RotatorValue = RowState.BaselineValue.RotatorValue;
		}
		break;
	default:
		break;
	}

	return Value;
}

FRuntimeGameplaySettingsSnapshot URuntimeGameplaySettingsWidget::BuildSnapshotFromRows() const
{
	FRuntimeGameplaySettingsSnapshot Snapshot;
	Snapshot.ProfilePath = FSoftObjectPath(Profile);

	for (const FRuntimeGameplaySettingsRowState& RowState : RowStates)
	{
		int32 ObjectIndex = Snapshot.Objects.IndexOfByPredicate([&RowState](const FRuntimeGameplaySettingsObjectSnapshot& ObjectSnapshot)
		{
			return ObjectSnapshot.TargetClassPath == RowState.TargetClassPath;
		});

		if (ObjectIndex == INDEX_NONE)
		{
			ObjectIndex = Snapshot.Objects.AddDefaulted();
			Snapshot.Objects[ObjectIndex].TargetClassPath = RowState.TargetClassPath;
		}

		FRuntimeGameplaySettingsPropertySnapshot PropertySnapshot;
		PropertySnapshot.bIsComponentProperty = RowState.PropertyEntry.bIsComponentProperty;
		PropertySnapshot.ComponentName = RowState.PropertyEntry.ComponentName;
		PropertySnapshot.ComponentClassPath = RowState.PropertyEntry.ComponentClassPath;
		PropertySnapshot.PropertyName = RowState.PropertyEntry.PropertyName;
		PropertySnapshot.PropertyPath = RowState.PropertyEntry.PropertyPath;
		PropertySnapshot.ValueType = RowState.PropertyEntry.ValueType;
		PropertySnapshot.EnumPath = RowState.PropertyEntry.EnumPath;
		PropertySnapshot.Value = GetRowValue(RowState);
		Snapshot.Objects[ObjectIndex].Properties.Add(PropertySnapshot);
	}

	return Snapshot;
}

void URuntimeGameplaySettingsWidget::RefreshSlotWidgets()
{
	CachedSlots.Reset();
	if (SettingsSubsystem)
	{
		CachedSlots = SettingsSubsystem->GetAvailableSlots();
	}

	const FString CurrentSlotName = SettingsSubsystem ? SettingsSubsystem->GetCurrentSlotDisplayName() : FString();
	const FString PreferredSlotName = !CurrentSlotName.IsEmpty()
		? CurrentSlotName
		: (CachedSlots.Num() > 0 ? CachedSlots[0].DisplayName : FString());

	TGuardValue<bool> SyncGuard(bIsSynchronizingSlots, true);

	if (SaveSlotPanelWidget)
	{
		SaveSlotPanelWidget->RefreshSlots(CachedSlots, PreferredSlotName, CurrentSlotName);
	}

	if (ButtonsOverlayWidget)
	{
		ButtonsOverlayWidget->SetCurrentSlotName(CurrentSlotName);
	}

	UpdateSlotActionStates();
}

const FRuntimeGameplaySettingsSlotDescriptor* URuntimeGameplaySettingsWidget::FindCachedSlotByDisplayName(
	const FString& DisplayName) const
{
	const FString SanitizedDisplayName = DisplayName.TrimStartAndEnd();
	if (SanitizedDisplayName.IsEmpty())
	{
		return nullptr;
	}

	return CachedSlots.FindByPredicate([&SanitizedDisplayName](const FRuntimeGameplaySettingsSlotDescriptor& CachedSlot)
	{
		return CachedSlot.DisplayName.Equals(SanitizedDisplayName, ESearchCase::IgnoreCase);
	});
}

FString URuntimeGameplaySettingsWidget::GetRequestedSlotName() const
{
	if (SaveSlotPanelWidget)
	{
		return SaveSlotPanelWidget->GetRequestedSlotName();
	}

	return FString();
}

FString URuntimeGameplaySettingsWidget::GetSelectedSlotName() const
{
	if (SaveSlotPanelWidget)
	{
		return SaveSlotPanelWidget->GetSelectedSlotName();
	}

	return FString();
}

void URuntimeGameplaySettingsWidget::SyncRequestedSlotName(const FString& SlotName)
{
	if (SaveSlotPanelWidget)
	{
		SaveSlotPanelWidget->SyncRequestedSlotName(SlotName);
	}
}

void URuntimeGameplaySettingsWidget::UpdateSlotActionStates()
{
	const bool bHasCurrentSlot = SettingsSubsystem && SettingsSubsystem->HasCurrentSlot();
	const bool bHasSelectedSlot = FindCachedSlotByDisplayName(GetSelectedSlotName()) != nullptr;
	const bool bHasRequestedSlotName = !GetRequestedSlotName().IsEmpty();

	if (ButtonsOverlayWidget)
	{
		ButtonsOverlayWidget->SetSaveEnabled(bHasCurrentSlot);
	}

	if (SaveSlotPanelWidget)
	{
		SaveSlotPanelWidget->SetActionStates(bHasRequestedSlotName, bHasSelectedSlot, bHasSelectedSlot);
	}
}

void URuntimeGameplaySettingsWidget::SetSaveSlotPanelVisible(bool bVisible)
{
	if (SaveSlotPanelWidget)
	{
		SaveSlotPanelWidget->SetPanelVisible(bVisible);
	}
}

bool URuntimeGameplaySettingsWidget::IsSaveSlotPanelVisible() const
{
	if (SaveSlotPanelWidget)
	{
		return SaveSlotPanelWidget->IsPanelVisible();
	}

	return false;
}

void URuntimeGameplaySettingsWidget::ActivateRuntimeCategory(int32 CategoryIndex)
{
	if (!CategoryStates.IsValidIndex(CategoryIndex))
	{
		return;
	}

	if (CategoryTabsWidget)
	{
		CategoryTabsWidget->ActivateCategory(CategoryIndex);
	}

	ActivateCategoryPage(CategoryIndex);
}

void URuntimeGameplaySettingsWidget::ClearCategoryPages()
{
	if (Switcher_DeveloperSettings)
	{
		Switcher_DeveloperSettings->ClearChildren();
	}
}

int32 URuntimeGameplaySettingsWidget::AddCategoryPage(UWidget* PageWidget)
{
	if (!PageWidget)
	{
		return INDEX_NONE;
	}

	if (Switcher_DeveloperSettings)
	{
		Switcher_DeveloperSettings->AddChild(PageWidget);
		return Switcher_DeveloperSettings->GetChildIndex(PageWidget);
	}

	return INDEX_NONE;
}

void URuntimeGameplaySettingsWidget::ActivateCategoryPage(int32 CategoryIndex)
{
	if (Switcher_DeveloperSettings
		&& CategoryIndex >= 0
		&& CategoryIndex < Switcher_DeveloperSettings->GetNumWidgets())
	{
		Switcher_DeveloperSettings->SetActiveWidgetIndex(CategoryIndex);
		return;
	}

}

void URuntimeGameplaySettingsWidget::SetCategoryTitleFromTargetClass(
	const FString& RuntimeCategoryName,
	UClass* TargetClass)
{
	const FString TargetClassName = GetTargetClassDisplayName(TargetClass);
	if (TargetClassName.IsEmpty())
	{
		return;
	}

	for (FRuntimeGameplaySettingsCategoryState& CategoryState : CategoryStates)
	{
		if (!CategoryState.RuntimeCategoryName.Equals(RuntimeCategoryName, ESearchCase::IgnoreCase))
		{
			continue;
		}

		for (const FString& ExistingClassName : CategoryState.TargetClassNames)
		{
			if (ExistingClassName.Equals(TargetClassName, ESearchCase::IgnoreCase))
			{
				return;
			}
		}

		CategoryState.TargetClassNames.Add(TargetClassName);

		FString TitleText;
		for (const FString& ClassName : CategoryState.TargetClassNames)
		{
			if (!TitleText.IsEmpty())
			{
				TitleText += TEXT(", ");
			}
			TitleText += ClassName;
		}

		if (CategoryState.PageWidget)
		{
			CategoryState.PageWidget->SetTitleText(FText::FromString(TitleText));
		}
		return;
	}
}

FString URuntimeGameplaySettingsWidget::GetTargetClassDisplayName(UClass* TargetClass) const
{
	if (!TargetClass)
	{
		return FString();
	}

	FString TargetClassName = TargetClass->GetName();
	TargetClassName.RemoveFromEnd(TEXT("_C"));
	return TargetClassName;
}

UTextBlock* URuntimeGameplaySettingsWidget::CreateTextBlock(const FText& Text)
{
	UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	TextBlock->SetText(Text);
	return TextBlock;
}

UWidget* URuntimeGameplaySettingsWidget::CreateToolTipWidget(const FText& InToolTipText)
{
	if (!WidgetTree || InToolTipText.IsEmpty())
	{
		return nullptr;
	}

	UBorder* ToolTipBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	UTextBlock* ToolTipTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	if (!ToolTipBorder || !ToolTipTextBlock)
	{
		return nullptr;
	}

	FSlateFontInfo ToolTipFont = ToolTipTextBlock->GetFont();
	ToolTipFont.Size = FMath::Max(
		1,
		FMath::RoundToInt(RuntimeGameplaySettingsBaseToolTipFontSize * RuntimeGameplaySettingsToolTipFontScale));
	ToolTipTextBlock->SetFont(ToolTipFont);
	ToolTipTextBlock->SetText(InToolTipText);
	ToolTipTextBlock->SetAutoWrapText(true);
	ToolTipTextBlock->SetWrapTextAt(700.0f);
	ToolTipTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));

	ToolTipBorder->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.9f));
	ToolTipBorder->SetPadding(FMargin(8.0f, 6.0f));
	ToolTipBorder->AddChild(ToolTipTextBlock);
	return ToolTipBorder;
}

void URuntimeGameplaySettingsWidget::ApplyConfiguredWidgetClasses()
{
	const URuntimeGameplaySettingsProjectSettings* Settings =
		GetDefault<URuntimeGameplaySettingsProjectSettings>();
	if (!Settings)
	{
		return;
	}

	SaveSlotPanelWidgetClass = ResolveConfiguredWidgetClass(
		Settings->SaveSlotPanelWidgetClass,
		SaveSlotPanelWidgetClass);
	ButtonsOverlayWidgetClass = ResolveConfiguredWidgetClass(
		Settings->ButtonsOverlayWidgetClass,
		ButtonsOverlayWidgetClass);
	CategoryTabsWidgetClass = ResolveConfiguredWidgetClass(
		Settings->CategoryTabsWidgetClass,
		CategoryTabsWidgetClass);
	CategoryPageWidgetClass = ResolveConfiguredWidgetClass(
		Settings->CategoryPageWidgetClass,
		CategoryPageWidgetClass);
	ComponentGroupWidgetClass = ResolveConfiguredWidgetClass(
		Settings->ComponentGroupWidgetClass,
		ComponentGroupWidgetClass);
	CategoryButtonWidgetClass = ResolveConfiguredWidgetClass(
		Settings->CategoryButtonWidgetClass,
		CategoryButtonWidgetClass);
	BoolPropertyWidgetClass = ResolveConfiguredWidgetClass(
		Settings->BoolPropertyWidgetClass,
		BoolPropertyWidgetClass);
	NumberPropertyWidgetClass = ResolveConfiguredWidgetClass(
		Settings->NumberPropertyWidgetClass,
		NumberPropertyWidgetClass);
	EnumPropertyWidgetClass = ResolveConfiguredWidgetClass(
		Settings->EnumPropertyWidgetClass,
		EnumPropertyWidgetClass);
	VectorPropertyWidgetClass = ResolveConfiguredWidgetClass(
		Settings->VectorPropertyWidgetClass,
		VectorPropertyWidgetClass);
	CurvePropertyWidgetClass = ResolveConfiguredWidgetClass(
		Settings->CurvePropertyWidgetClass,
		CurvePropertyWidgetClass);
	CurveAssetPropertyWidgetClass = ResolveConfiguredWidgetClass(
		Settings->CurveAssetPropertyWidgetClass,
		CurveAssetPropertyWidgetClass);
	FloatArrayPropertyWidgetClass = ResolveConfiguredWidgetClass(
		Settings->FloatArrayPropertyWidgetClass,
		FloatArrayPropertyWidgetClass);
}

void URuntimeGameplaySettingsWidget::EnsureDefaultWidgetTree()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	BuildDefaultWidgetTree();
}

void URuntimeGameplaySettingsWidget::BuildDefaultWidgetTree()
{
	if (!WidgetTree)
	{
		return;
	}

	ApplyConfiguredWidgetClasses();

	UCanvasPanel* CanvasPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("CanvasPanel"));
	UImage* BackgroundImg = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("BackgroundImg"));
	UHorizontalBox* HorizontalBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("HorizontalBox"));
	UImage* VLineImg = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("VLineImg"));
	UVerticalBox* VerticalBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("VerticalBox"));

	SaveSlotPanelWidget = WidgetTree->ConstructWidget<URuntimeGameplaySettingsSaveSlotPanelWidget>(
		SaveSlotPanelWidgetClass ? SaveSlotPanelWidgetClass.Get() : URuntimeGameplaySettingsSaveSlotPanelWidget::StaticClass(),
		TEXT("SaveSlotPanelWidget"));
	ButtonsOverlayWidget = WidgetTree->ConstructWidget<URuntimeGameplaySettingsButtonsOverlayWidget>(
		ButtonsOverlayWidgetClass ? ButtonsOverlayWidgetClass.Get() : URuntimeGameplaySettingsButtonsOverlayWidget::StaticClass(),
		TEXT("ButtonsOverlayWidget"));
	CategoryTabsWidget = WidgetTree->ConstructWidget<URuntimeGameplaySettingsCategoryTabsWidget>(
		CategoryTabsWidgetClass ? CategoryTabsWidgetClass.Get() : URuntimeGameplaySettingsCategoryTabsWidget::StaticClass(),
		TEXT("CategoryTabsWidget"));
	Switcher_DeveloperSettings = WidgetTree->ConstructWidget<UWidgetSwitcher>(
		UWidgetSwitcher::StaticClass(),
		TEXT("Switcher_DeveloperSettings"));

	if (!CanvasPanel
		|| !BackgroundImg
		|| !HorizontalBox
		|| !VLineImg
		|| !VerticalBox
		|| !CategoryTabsWidget
		|| !ButtonsOverlayWidget
		|| !Switcher_DeveloperSettings
		|| !SaveSlotPanelWidget)
	{
		return;
	}

	SaveSlotPanelWidget->Initialize();
	ButtonsOverlayWidget->Initialize();
	CategoryTabsWidget->Initialize();

	BackgroundImg->SetColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 217.0f / 255.0f));
	if (UCanvasPanelSlot* BackgroundSlot = CanvasPanel->AddChildToCanvas(BackgroundImg))
	{
		BackgroundSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		BackgroundSlot->SetOffsets(FMargin(0.0f));
		BackgroundSlot->SetZOrder(0);
	}

	if (UHorizontalBoxSlot* CategorySlot = HorizontalBox->AddChildToHorizontalBox(CategoryTabsWidget))
	{
		CategorySlot->SetPadding(FMargin(0.0f, 20.0f, 0.0f, 20.0f));
		CategorySlot->SetHorizontalAlignment(HAlign_Left);
		CategorySlot->SetVerticalAlignment(VAlign_Fill);
	}

	FSlateBrush VLineBrush = VLineImg->GetBrush();
	VLineBrush.DrawAs = ESlateBrushDrawType::Image;
	VLineBrush.ImageSize = FVector2D(3.0f, 3.0f);
	VLineImg->SetBrush(VLineBrush);
	VLineImg->SetDesiredSizeOverride(FVector2D(3.0f, 3.0f));
	VLineImg->SetColorAndOpacity(FLinearColor(0.403922f, 0.403922f, 0.403922f, 1.0f));
	if (UHorizontalBoxSlot* VLineSlot = HorizontalBox->AddChildToHorizontalBox(VLineImg))
	{
		VLineSlot->SetPadding(FMargin(10.0f, 0.0f, 10.0f, 0.0f));
		VLineSlot->SetHorizontalAlignment(HAlign_Center);
		VLineSlot->SetVerticalAlignment(VAlign_Fill);
	}

	if (UVerticalBoxSlot* SwitcherSlot = VerticalBox->AddChildToVerticalBox(Switcher_DeveloperSettings))
	{
		SwitcherSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 20.0f));
		SwitcherSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		SwitcherSlot->SetHorizontalAlignment(HAlign_Fill);
		SwitcherSlot->SetVerticalAlignment(VAlign_Fill);
	}

	if (UVerticalBoxSlot* ButtonsSlot = VerticalBox->AddChildToVerticalBox(ButtonsOverlayWidget))
	{
		ButtonsSlot->SetHorizontalAlignment(HAlign_Fill);
		ButtonsSlot->SetVerticalAlignment(VAlign_Bottom);
	}

	if (UHorizontalBoxSlot* VerticalBoxSlot = HorizontalBox->AddChildToHorizontalBox(VerticalBox))
	{
		VerticalBoxSlot->SetPadding(FMargin(10.0f, 20.0f, 20.0f, 10.0f));
		VerticalBoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		VerticalBoxSlot->SetHorizontalAlignment(HAlign_Fill);
		VerticalBoxSlot->SetVerticalAlignment(VAlign_Fill);
	}

	if (UCanvasPanelSlot* HorizontalBoxSlot = CanvasPanel->AddChildToCanvas(HorizontalBox))
	{
		HorizontalBoxSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		HorizontalBoxSlot->SetOffsets(FMargin(0.0f));
		HorizontalBoxSlot->SetAutoSize(true);
		HorizontalBoxSlot->SetZOrder(110);
	}

	if (UCanvasPanelSlot* SaveSlot = CanvasPanel->AddChildToCanvas(SaveSlotPanelWidget))
	{
		SaveSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		SaveSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		SaveSlot->SetAutoSize(true);
		SaveSlot->SetZOrder(200);
	}

	WidgetTree->RootWidget = CanvasPanel;
}

FString URuntimeGameplaySettingsWidget::MakeUniqueWidgetName(
	const FString& Prefix,
	const FString& SourceText,
	TSet<FString>& UsedNames) const
{
	const FString SanitizedSource = SanitizeWidgetObjectName(SourceText);
	const FString BaseName = Prefix + TEXT("_") + SanitizedSource;
	FString CandidateName = BaseName;
	int32 Suffix = 2;
	while (UsedNames.Contains(CandidateName))
	{
		CandidateName = FString::Printf(TEXT("%s_%d"), *BaseName, Suffix);
		++Suffix;
	}

	UsedNames.Add(CandidateName);
	return CandidateName;
}

FString URuntimeGameplaySettingsWidget::SanitizeWidgetObjectName(const FString& SourceText) const
{
	FString Result;
	Result.Reserve(SourceText.Len());

	bool bLastWasUnderscore = false;
	for (const TCHAR Character : SourceText)
	{
		const bool bIsAsciiLetter =
			(Character >= TCHAR('A') && Character <= TCHAR('Z'))
			|| (Character >= TCHAR('a') && Character <= TCHAR('z'));
		const bool bIsDigit = Character >= TCHAR('0') && Character <= TCHAR('9');
		const bool bCanUseCharacter = bIsAsciiLetter || bIsDigit || Character == TCHAR('_');

		if (bCanUseCharacter)
		{
			Result.AppendChar(Character);
			bLastWasUnderscore = false;
		}
		else if (!bLastWasUnderscore)
		{
			Result.AppendChar(TCHAR('_'));
			bLastWasUnderscore = true;
		}
	}

	Result.TrimStartAndEndInline();
	while (Result.StartsWith(TEXT("_")))
	{
		Result.RightChopInline(1, EAllowShrinking::No);
	}
	while (Result.EndsWith(TEXT("_")))
	{
		Result.LeftChopInline(1, EAllowShrinking::No);
	}

	return Result.IsEmpty() ? TEXT("Unnamed") : Result;
}

void URuntimeGameplaySettingsWidget::HandleApplyClicked()
{
	ApplyRuntimeGameplaySettingsToTargets();
}

void URuntimeGameplaySettingsWidget::HandleSaveClicked()
{
	if (!SettingsSubsystem)
	{
		return;
	}

	const FRuntimeGameplaySettingsSnapshot Snapshot = BuildSnapshotFromRows();
	if (SettingsSubsystem->SaveCurrent(Snapshot))
	{
		ApplyRuntimeGameplaySettingsToTargets();
		RefreshSlotWidgets();
	}
}

void URuntimeGameplaySettingsWidget::HandleSaveAsClicked()
{
	if (!SettingsSubsystem)
	{
		return;
	}

	const FRuntimeGameplaySettingsSnapshot Snapshot = BuildSnapshotFromRows();
	FRuntimeGameplaySettingsSlotDescriptor SavedSlot;
	if (SettingsSubsystem->SaveAs(GetRequestedSlotName(), Snapshot, SavedSlot))
	{
		ApplyRuntimeGameplaySettingsToTargets();
		RefreshSlotWidgets();
	}
}

void URuntimeGameplaySettingsWidget::HandleShowSaveAsClicked()
{
	SetSaveSlotPanelVisible(!IsSaveSlotPanelVisible());
}

void URuntimeGameplaySettingsWidget::HandleCloseSaveAsClicked()
{
	SetSaveSlotPanelVisible(false);
}

void URuntimeGameplaySettingsWidget::HandleLoadClicked()
{
	if (!SettingsSubsystem)
	{
		return;
	}

	const FRuntimeGameplaySettingsSlotDescriptor* SelectedSlot = FindCachedSlotByDisplayName(GetSelectedSlotName());
	if (!SelectedSlot)
	{
		return;
	}

	FRuntimeGameplaySettingsSnapshot Snapshot;
	if (SettingsSubsystem->LoadSlot(SelectedSlot->SlotId, Snapshot))
	{
		FRuntimeGameplaySettingsSnapshotLibrary::ApplySnapshot(RuntimePlayerController.Get(), Profile, Snapshot);
		RebuildRuntimeWidget();
	}
}

void URuntimeGameplaySettingsWidget::HandleDeleteClicked()
{
	if (!SettingsSubsystem)
	{
		return;
	}

	const FRuntimeGameplaySettingsSlotDescriptor* SelectedSlot = FindCachedSlotByDisplayName(GetSelectedSlotName());
	if (SelectedSlot && SettingsSubsystem->DeleteSlot(SelectedSlot->SlotId))
	{
		RefreshSlotWidgets();
	}
}

void URuntimeGameplaySettingsWidget::HandleResetAllClicked()
{
	if (!SettingsSubsystem)
	{
		return;
	}

	FRuntimeGameplaySettingsSnapshot BaselineSnapshot;
	if (SettingsSubsystem->TryGetBaselineSnapshot(BaselineSnapshot))
	{
		FRuntimeGameplaySettingsSnapshotLibrary::ApplySnapshot(RuntimePlayerController.Get(), Profile, BaselineSnapshot);
		RebuildRuntimeWidget();
	}
}

void URuntimeGameplaySettingsWidget::HandleCloseClicked()
{
	CloseRuntimeGameplaySettingsWidget();
}

void URuntimeGameplaySettingsWidget::HandleSlotSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	(void)SelectionType;
	if (!bIsSynchronizingSlots)
	{
		SyncRequestedSlotName(SelectedItem);
	}
	UpdateSlotActionStates();
}

void URuntimeGameplaySettingsWidget::HandleSlotNameChanged(const FText& InText)
{
	(void)InText;
	if (!bIsSynchronizingSlots)
	{
		UpdateSlotActionStates();
	}
}

void URuntimeGameplaySettingsWidget::HandleCategoryTabReleased(
	URuntimeGameplaySettingsButtonWidget* ButtonWidget)
{
	const int32 CategoryIndex = CategoryStates.IndexOfByPredicate(
		[ButtonWidget](const FRuntimeGameplaySettingsCategoryState& CategoryState)
		{
			return CategoryState.TabButton == ButtonWidget;
		});

	ActivateRuntimeCategory(CategoryIndex);
}

void URuntimeGameplaySettingsWidget::HandleCategorySelected(int32 CategoryIndex)
{
	ActivateRuntimeCategory(CategoryIndex);
}
