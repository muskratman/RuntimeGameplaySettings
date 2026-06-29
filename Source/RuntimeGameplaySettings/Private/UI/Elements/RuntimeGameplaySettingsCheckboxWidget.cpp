#include "UI/Elements/RuntimeGameplaySettingsCheckboxWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CheckBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Templates/UnrealTemplate.h"
#include "UI/Elements/RuntimeGameplaySettingsElementWidgetHelpers.h"

void URuntimeGameplaySettingsCheckboxWidget::SetCheckboxValue(bool bInValue)
{
	bCurrentValue = bInValue;
	bHasInitializedValue = true;
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsCheckboxWidget::SetDefaultCheckboxValue(bool bInValue)
{
	bRuntimeDefaultValue = bInValue;
	bHasRuntimeDefaultValue = true;
	UpdateResetButtonState();
}

bool URuntimeGameplaySettingsCheckboxWidget::GetCheckboxValue() const
{
	if (Checkbox_Value)
	{
		return Checkbox_Value->IsChecked();
	}

	return bCurrentValue;
}

void URuntimeGameplaySettingsCheckboxWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!bHasInitializedValue)
	{
		bCurrentValue = bDefaultValue;
	}
	if (!bHasRuntimeDefaultValue)
	{
		bRuntimeDefaultValue = bDefaultValue;
	}

	ApplyDisplayValues();
}

void URuntimeGameplaySettingsCheckboxWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Checkbox_Value)
	{
		Checkbox_Value->OnCheckStateChanged.AddDynamic(this, &URuntimeGameplaySettingsCheckboxWidget::HandleCheckboxValueChanged);
	}
	bCurrentValue = bDefaultValue;
	bHasInitializedValue = true;
	if (!bHasRuntimeDefaultValue)
	{
		bRuntimeDefaultValue = bDefaultValue;
	}
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsCheckboxWidget::ResetValueToDefault()
{
	SetCheckboxValue(GetDefaultCheckboxValue());
}

bool URuntimeGameplaySettingsCheckboxWidget::IsValueAtDefault() const
{
	return GetCheckboxValue() == GetDefaultCheckboxValue();
}

void URuntimeGameplaySettingsCheckboxWidget::BuildDefaultWidgetTree()
{
	using namespace RuntimeGameplaySettingsElementWidgetHelpers;

	if (!WidgetTree)
	{
		return;
	}

	UHorizontalBox* RootBox = ConstructWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_2"));
	Txt_Name = CreateTextBlock(WidgetTree, TEXT("Txt_Name"), INVTEXT("Param Name"), LabelFontSize);
	Checkbox_Value = ConstructWidget<UCheckBox>(WidgetTree, TEXT("Checkbox_Value"));
	ResetButtonWidget = CreateResetButtonWidget(WidgetTree);
	ApplyCheckBoxImageSize(Checkbox_Value, FVector2D(50.0f, 50.0f));

	if (!RootBox)
	{
		return;
	}

	if (UHorizontalBoxSlot* NameSlot = RootBox->AddChildToHorizontalBox(Txt_Name))
	{
		NameSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		NameSlot->SetPadding(FMargin(0.0f, 0.0f, 100.0f, 0.0f));
		NameSlot->SetHorizontalAlignment(HAlign_Left);
		NameSlot->SetVerticalAlignment(VAlign_Center);
	}
	AddToHorizontalBox(RootBox, Checkbox_Value, FMargin(0.0f), HAlign_Left, VAlign_Center);
	AddToHorizontalBox(RootBox, ResetButtonWidget, FMargin(10.0f, 0.0f, 0.0f, 0.0f), HAlign_Right);

	WidgetTree->RootWidget = RootBox;
}

void URuntimeGameplaySettingsCheckboxWidget::HandleCheckboxValueChanged(bool bIsChecked)
{
	if (bIsSynchronizing)
	{
		return;
	}

	bCurrentValue = bIsChecked;
	bHasInitializedValue = true;
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsCheckboxWidget::ApplyDisplayValues()
{
	TGuardValue<bool> SynchronizationGuard(bIsSynchronizing, true);

	if (Checkbox_Value)
	{
		Checkbox_Value->SetIsChecked(bCurrentValue);
	}

	Super::ApplyDisplayValues();
}

bool URuntimeGameplaySettingsCheckboxWidget::GetDefaultCheckboxValue() const
{
	return bHasRuntimeDefaultValue ? bRuntimeDefaultValue : bDefaultValue;
}
