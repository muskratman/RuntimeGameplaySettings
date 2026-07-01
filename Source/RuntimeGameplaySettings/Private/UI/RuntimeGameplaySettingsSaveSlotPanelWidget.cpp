#include "UI/RuntimeGameplaySettingsSaveSlotPanelWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Templates/UnrealTemplate.h"
#include "UI/Elements/RuntimeGameplaySettingsElementWidgetHelpers.h"

void URuntimeGameplaySettingsSaveSlotPanelWidget::SetPanelVisible(bool bVisible)
{
	const ESlateVisibility DesiredVisibility = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	if (Border_SaveSlot)
	{
		Border_SaveSlot->SetVisibility(DesiredVisibility);
	}
	else
	{
		SetVisibility(DesiredVisibility);
	}
}

bool URuntimeGameplaySettingsSaveSlotPanelWidget::IsPanelVisible() const
{
	return Border_SaveSlot
		? Border_SaveSlot->GetVisibility() != ESlateVisibility::Collapsed
		: GetVisibility() != ESlateVisibility::Collapsed;
}

void URuntimeGameplaySettingsSaveSlotPanelWidget::RefreshSlots(
	const TArray<FRuntimeGameplaySettingsSlotDescriptor>& Slots,
	const FString& PreferredSlotName,
	const FString& RequestedSlotName)
{
	TGuardValue<bool> SyncGuard(bIsSynchronizingSlots, true);

	if (Combo_Slots)
	{
		Combo_Slots->ClearOptions();
		for (const FRuntimeGameplaySettingsSlotDescriptor& SlotDescriptor : Slots)
		{
			Combo_Slots->AddOption(SlotDescriptor.DisplayName);
		}

		if (!PreferredSlotName.IsEmpty())
		{
			Combo_Slots->SetSelectedOption(PreferredSlotName);
		}
		else
		{
			Combo_Slots->ClearSelection();
		}
	}

	SyncRequestedSlotName(RequestedSlotName);
}

void URuntimeGameplaySettingsSaveSlotPanelWidget::SetActionStates(
	bool bCanSaveAs,
	bool bCanLoad,
	bool bCanDelete)
{
	if (Butt_SaveAs)
	{
		Butt_SaveAs->SetIsEnabled(bCanSaveAs);
	}
	if (Butt_Load)
	{
		Butt_Load->SetIsEnabled(bCanLoad);
	}
	if (Butt_Delete)
	{
		Butt_Delete->SetIsEnabled(bCanDelete);
	}
}

void URuntimeGameplaySettingsSaveSlotPanelWidget::SyncRequestedSlotName(const FString& SlotName)
{
	if (Editable_SlotName)
	{
		Editable_SlotName->SetText(FText::FromString(SlotName));
	}
}

FString URuntimeGameplaySettingsSaveSlotPanelWidget::GetRequestedSlotName() const
{
	return Editable_SlotName ? Editable_SlotName->GetText().ToString().TrimStartAndEnd() : FString();
}

FString URuntimeGameplaySettingsSaveSlotPanelWidget::GetSelectedSlotName() const
{
	return Combo_Slots ? Combo_Slots->GetSelectedOption().TrimStartAndEnd() : FString();
}

void URuntimeGameplaySettingsSaveSlotPanelWidget::NativePreConstruct()
{
	EnsureDefaultWidgetTree();
	Super::NativePreConstruct();
}

void URuntimeGameplaySettingsSaveSlotPanelWidget::NativeOnInitialized()
{
	EnsureDefaultWidgetTree();
	Super::NativeOnInitialized();

	if (Button_CloseSaveAs)
	{
		Button_CloseSaveAs->OnClicked.AddUniqueDynamic(this, &URuntimeGameplaySettingsSaveSlotPanelWidget::HandleCloseClicked);
	}
	if (Butt_SaveAs)
	{
		Butt_SaveAs->OnClicked.AddUniqueDynamic(this, &URuntimeGameplaySettingsSaveSlotPanelWidget::HandleSaveAsClicked);
	}
	if (Butt_Load)
	{
		Butt_Load->OnClicked.AddUniqueDynamic(this, &URuntimeGameplaySettingsSaveSlotPanelWidget::HandleLoadClicked);
	}
	if (Butt_Delete)
	{
		Butt_Delete->OnClicked.AddUniqueDynamic(this, &URuntimeGameplaySettingsSaveSlotPanelWidget::HandleDeleteClicked);
	}
	if (Combo_Slots)
	{
		Combo_Slots->OnSelectionChanged.AddUniqueDynamic(
			this,
			&URuntimeGameplaySettingsSaveSlotPanelWidget::HandleSlotSelectionChanged);
	}
	if (Editable_SlotName)
	{
		Editable_SlotName->OnTextChanged.AddUniqueDynamic(
			this,
			&URuntimeGameplaySettingsSaveSlotPanelWidget::HandleSlotNameChanged);
	}
}

void URuntimeGameplaySettingsSaveSlotPanelWidget::HandleCloseClicked()
{
	OnCloseRequested.Broadcast();
}

void URuntimeGameplaySettingsSaveSlotPanelWidget::HandleSaveAsClicked()
{
	OnSaveAsRequested.Broadcast();
}

void URuntimeGameplaySettingsSaveSlotPanelWidget::HandleLoadClicked()
{
	OnLoadRequested.Broadcast();
}

void URuntimeGameplaySettingsSaveSlotPanelWidget::HandleDeleteClicked()
{
	OnDeleteRequested.Broadcast();
}

void URuntimeGameplaySettingsSaveSlotPanelWidget::HandleSlotSelectionChanged(
	FString SelectedItem,
	ESelectInfo::Type SelectionType)
{
	if (!bIsSynchronizingSlots)
	{
		OnSlotSelectionChanged.Broadcast(SelectedItem, SelectionType);
	}
}

void URuntimeGameplaySettingsSaveSlotPanelWidget::HandleSlotNameChanged(const FText& InText)
{
	if (!bIsSynchronizingSlots)
	{
		OnSlotNameChanged.Broadcast(InText);
	}
}

void URuntimeGameplaySettingsSaveSlotPanelWidget::EnsureDefaultWidgetTree()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, UWidgetTree::StaticClass(), TEXT("WidgetTree"), RF_Transient);
	}

	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	BuildDefaultWidgetTree();
}

void URuntimeGameplaySettingsSaveSlotPanelWidget::BuildDefaultWidgetTree()
{
	using namespace RuntimeGameplaySettingsElementWidgetHelpers;

	if (!WidgetTree)
	{
		return;
	}

	Border_SaveSlot = ConstructWidget<UBorder>(WidgetTree, TEXT("Border_SaveSlot"));
	UVerticalBox* RootBox = ConstructWidget<UVerticalBox>(WidgetTree, TEXT("VerticalBox_3"));
	UVerticalBox* SaveSlotBox = ConstructWidget<UVerticalBox>(WidgetTree, TEXT("VBox_SaveSlot"));
	UHorizontalBox* ActionBox = ConstructWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_195"));

	Button_CloseSaveAs = CreateTextButton(
		WidgetTree,
		TEXT("Button_CloseSaveAs"),
		TEXT("TextBlock_8"),
		INVTEXT("X"),
		FLinearColor(0.856771f, 0.000052f, 0.012691f, 1.0f),
		FLinearColor(0.343750f, 0.000021f, 0.005092f, 1.0f),
		30.0f);
	Butt_SaveAs = CreateTextButton(
		WidgetTree,
		TEXT("Butt_SaveAs"),
		TEXT("TextBlock_5"),
		INVTEXT("Save As"),
		FLinearColor(0.0f, 0.614583f, 0.381093f, 1.0f),
		FLinearColor(0.0f, 0.294271f, 0.182473f, 1.0f),
		40.0f);
	Butt_Load = CreateTextButton(
		WidgetTree,
		TEXT("Butt_Load"),
		TEXT("TextBlock_6"),
		INVTEXT("Load"),
		FLinearColor(0.674479f, 0.584826f, 0.0f, 1.0f),
		FLinearColor(0.351562f, 0.304832f, 0.0f, 1.0f),
		40.0f);
	Butt_Delete = CreateTextButton(
		WidgetTree,
		TEXT("Butt_Delete"),
		TEXT("TextBlock_7"),
		INVTEXT("Delete"),
		FLinearColor(0.674479f, 0.584826f, 0.0f, 1.0f),
		FLinearColor(0.351562f, 0.304832f, 0.0f, 1.0f),
		40.0f);
	Combo_Slots = ConstructWidget<UComboBoxString>(WidgetTree, TEXT("Combo_Slots"));
	USizeBox* ComboSizeBox = ConstructWidget<USizeBox>(WidgetTree, TEXT("SizeBox_1"));
	Editable_SlotName = ConstructWidget<UEditableTextBox>(WidgetTree, TEXT("Editable_SlotName"));
	USizeBox* EditableSizeBox = ConstructWidget<USizeBox>(WidgetTree, TEXT("SizeBox_0"));

	if (!Border_SaveSlot || !RootBox || !SaveSlotBox || !ActionBox || !ComboSizeBox || !EditableSizeBox)
	{
		return;
	}

	SetButtonContentPadding(Button_CloseSaveAs, FMargin(10.0f, 5.0f, 10.0f, 5.0f));
	SetButtonContentPadding(Butt_SaveAs, FMargin(10.0f));
	SetButtonContentPadding(Butt_Load, FMargin(10.0f));
	SetButtonContentPadding(Butt_Delete, FMargin(10.0f));

	if (Combo_Slots)
	{
		ComboSizeBox->AddChild(Combo_Slots);
	}
	if (Editable_SlotName)
	{
		ApplyEditableTextBoxStyle(Editable_SlotName, 30.0f);
		Editable_SlotName->SetHintText(INVTEXT("Slot Name"));
		EditableSizeBox->AddChild(Editable_SlotName);
	}

	AddToHorizontalBox(ActionBox, Butt_SaveAs, FMargin(50.0f, 0.0f), HAlign_Center, VAlign_Top);
	AddToHorizontalBox(ActionBox, Butt_Load, FMargin(50.0f, 0.0f), HAlign_Center, VAlign_Top);
	AddToHorizontalBox(ActionBox, Butt_Delete, FMargin(50.0f, 0.0f), HAlign_Center, VAlign_Top);

	if (UVerticalBoxSlot* ActionSlot = SaveSlotBox->AddChildToVerticalBox(ActionBox))
	{
		ActionSlot->SetHorizontalAlignment(HAlign_Center);
	}
	if (UVerticalBoxSlot* ComboSlot = SaveSlotBox->AddChildToVerticalBox(ComboSizeBox))
	{
		ComboSlot->SetPadding(FMargin(0.0f, 20.0f, 0.0f, 20.0f));
		ComboSlot->SetHorizontalAlignment(HAlign_Center);
		ComboSlot->SetVerticalAlignment(VAlign_Center);
	}
	if (UVerticalBoxSlot* EditableSlot = SaveSlotBox->AddChildToVerticalBox(EditableSizeBox))
	{
		EditableSlot->SetHorizontalAlignment(HAlign_Center);
		EditableSlot->SetVerticalAlignment(VAlign_Center);
	}

	if (UVerticalBoxSlot* CloseSlot = RootBox->AddChildToVerticalBox(Button_CloseSaveAs))
	{
		CloseSlot->SetPadding(FMargin(0.0f, 20.0f, 20.0f, 0.0f));
		CloseSlot->SetHorizontalAlignment(HAlign_Right);
		CloseSlot->SetVerticalAlignment(VAlign_Top);
	}
	if (UVerticalBoxSlot* SaveSlot = RootBox->AddChildToVerticalBox(SaveSlotBox))
	{
		SaveSlot->SetPadding(FMargin(100.0f, 50.0f, 100.0f, 100.0f));
	}

	Border_SaveSlot->SetHorizontalAlignment(HAlign_Center);
	Border_SaveSlot->SetVerticalAlignment(VAlign_Center);
	Border_SaveSlot->SetPadding(FMargin(0.0f));
	Border_SaveSlot->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 216.0f / 255.0f));
	Border_SaveSlot->SetVisibility(ESlateVisibility::Collapsed);
	Border_SaveSlot->AddChild(RootBox);
	WidgetTree->RootWidget = Border_SaveSlot;
}
