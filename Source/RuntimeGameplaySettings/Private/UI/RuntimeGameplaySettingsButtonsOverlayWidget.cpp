#include "UI/RuntimeGameplaySettingsButtonsOverlayWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "UI/Elements/RuntimeGameplaySettingsElementWidgetHelpers.h"

void URuntimeGameplaySettingsButtonsOverlayWidget::SetCurrentSlotName(const FString& SlotName)
{
	EnsureDefaultWidgetTree();
	if (Txt_CurrSlotName)
	{
		Txt_CurrSlotName->SetText(FText::FromString(SlotName.IsEmpty() ? TEXT("No current slot") : SlotName));
	}
}

void URuntimeGameplaySettingsButtonsOverlayWidget::SetSaveEnabled(bool bCanSave)
{
	EnsureDefaultWidgetTree();
	if (Butt_Save)
	{
		Butt_Save->SetIsEnabled(bCanSave);
	}
}

void URuntimeGameplaySettingsButtonsOverlayWidget::NativePreConstruct()
{
	EnsureDefaultWidgetTree();
	Super::NativePreConstruct();
}

void URuntimeGameplaySettingsButtonsOverlayWidget::NativeOnInitialized()
{
	EnsureDefaultWidgetTree();
	Super::NativeOnInitialized();

	if (Butt_ShowSaveAs)
	{
		Butt_ShowSaveAs->OnClicked.AddUniqueDynamic(
			this,
			&URuntimeGameplaySettingsButtonsOverlayWidget::HandleShowSaveAsClicked);
	}
	if (Butt_Save)
	{
		Butt_Save->OnClicked.AddUniqueDynamic(this, &URuntimeGameplaySettingsButtonsOverlayWidget::HandleSaveClicked);
	}
	if (Butt_Close)
	{
		Butt_Close->OnClicked.AddUniqueDynamic(this, &URuntimeGameplaySettingsButtonsOverlayWidget::HandleCloseClicked);
	}
	if (Butt_ResetAllToDefault)
	{
		Butt_ResetAllToDefault->OnClicked.AddUniqueDynamic(
			this,
			&URuntimeGameplaySettingsButtonsOverlayWidget::HandleResetAllToDefaultClicked);
	}
}

void URuntimeGameplaySettingsButtonsOverlayWidget::HandleShowSaveAsClicked()
{
	OnShowSaveAsRequested.Broadcast();
}

void URuntimeGameplaySettingsButtonsOverlayWidget::HandleSaveClicked()
{
	OnSaveRequested.Broadcast();
}

void URuntimeGameplaySettingsButtonsOverlayWidget::HandleCloseClicked()
{
	OnCloseRequested.Broadcast();
}

void URuntimeGameplaySettingsButtonsOverlayWidget::HandleResetAllToDefaultClicked()
{
	OnResetAllToDefaultRequested.Broadcast();
}

void URuntimeGameplaySettingsButtonsOverlayWidget::EnsureDefaultWidgetTree()
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

void URuntimeGameplaySettingsButtonsOverlayWidget::BuildDefaultWidgetTree()
{
	using namespace RuntimeGameplaySettingsElementWidgetHelpers;

	if (!WidgetTree)
	{
		return;
	}

	UOverlay* ButtonsOverlay = ConstructWidget<UOverlay>(WidgetTree, TEXT("ButtonsOverlay"));
	UHorizontalBox* CurrentSlotBox = ConstructWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_3"));
	UHorizontalBox* ButtonsBox = ConstructWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_4"));
	UTextBlock* CurrSlotText = CreateTextBlock(WidgetTree, TEXT("CurrSlotTxt"), INVTEXT("Curr Slot: "), 27.0f);
	Txt_CurrSlotName = CreateTextBlock(WidgetTree, TEXT("Txt_CurrSlotName"), INVTEXT("No current slot"), 27.0f);
	Butt_ShowSaveAs = CreateTextButton(
		WidgetTree,
		TEXT("Butt_ShowSaveAs"),
		TEXT("TextBlock_102"),
		INVTEXT("Save/Load"),
		FLinearColor(0.846354f, 0.537429f, 0.0f, 1.0f),
		FLinearColor(0.609375f, 0.386949f, 0.0f, 1.0f),
		27.0f);
	Butt_Save = CreateTextButton(
		WidgetTree,
		TEXT("Butt_Save"),
		TEXT("TextBlock_3"),
		INVTEXT("Save"),
		FLinearColor(0.0f, 0.495466f, 0.118741f, 1.0f),
		FLinearColor(0.0f, 0.171875f, 0.041191f, 1.0f),
		27.0f);
	Butt_ResetAllToDefault = CreateTextButton(
		WidgetTree,
		TEXT("Butt_ResetAllToDefault"),
		TEXT("TextBlock"),
		INVTEXT("Reset All to Default"),
		ResetNormalTint(),
		ResetPressedTint(),
		27.0f);
	Butt_Close = CreateTextButton(
		WidgetTree,
		TEXT("Butt_Close"),
		TEXT("TextBlock_4"),
		INVTEXT("Close"),
		FLinearColor(0.495466f, 0.063409f, 0.041289f, 1.0f),
		FLinearColor(0.263021f, 0.033661f, 0.021918f, 1.0f),
		27.0f);

	if (!ButtonsOverlay || !CurrentSlotBox || !ButtonsBox)
	{
		return;
	}

	SetButtonContentPadding(Butt_ShowSaveAs, FMargin(10.0f));
	SetButtonContentPadding(Butt_Save, FMargin(10.0f));
	SetButtonContentPadding(Butt_Close, FMargin(10.0f));
	SetButtonContentPadding(Butt_ResetAllToDefault, FMargin(10.0f));

	AddToHorizontalBox(CurrentSlotBox, CurrSlotText, FMargin(0.0f), HAlign_Left, VAlign_Center);
	AddToHorizontalBox(CurrentSlotBox, Txt_CurrSlotName, FMargin(0.0f), HAlign_Left, VAlign_Center);

	AddToHorizontalBox(ButtonsBox, Butt_ShowSaveAs, FMargin(10.0f, 0.0f, 10.0f, 0.0f));
	AddToHorizontalBox(ButtonsBox, Butt_Save, FMargin(10.0f, 0.0f, 10.0f, 0.0f));
	AddToHorizontalBox(ButtonsBox, Butt_Close, FMargin(10.0f, 0.0f, 10.0f, 0.0f));

	if (UOverlaySlot* CurrentSlot = ButtonsOverlay->AddChildToOverlay(CurrentSlotBox))
	{
		CurrentSlot->SetVerticalAlignment(VAlign_Center);
	}
	if (UOverlaySlot* ButtonsSlot = ButtonsOverlay->AddChildToOverlay(ButtonsBox))
	{
		ButtonsSlot->SetHorizontalAlignment(HAlign_Center);
		ButtonsSlot->SetVerticalAlignment(VAlign_Center);
	}
	if (UOverlaySlot* ResetSlot = ButtonsOverlay->AddChildToOverlay(Butt_ResetAllToDefault))
	{
		ResetSlot->SetHorizontalAlignment(HAlign_Right);
		ResetSlot->SetVerticalAlignment(VAlign_Center);
	}

	WidgetTree->RootWidget = ButtonsOverlay;
}
