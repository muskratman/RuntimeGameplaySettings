#include "UI/Elements/RuntimeGameplaySettingsFloatArrayElementWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Containers/AllowShrinking.h"
#include "Misc/DefaultValueHelper.h"
#include "Templates/UnrealTemplate.h"
#include "UI/Elements/RuntimeGameplaySettingsElementWidgetHelpers.h"

void URuntimeGameplaySettingsFloatArrayElementWidget::SetFloatValue(float InValue)
{
	CurrentValue = InValue;
	bHasInitializedValue = true;
	ApplyDisplayValues();
}

float URuntimeGameplaySettingsFloatArrayElementWidget::GetFloatValue() const
{
	if (Editable_Value)
	{
		float ParsedValue = 0.0f;
		if (TryParseValue(Editable_Value->GetText(), ParsedValue))
		{
			return ParsedValue;
		}
	}

	return CurrentValue;
}

void URuntimeGameplaySettingsFloatArrayElementWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!bHasInitializedValue)
	{
		CurrentValue = DefaultValue;
	}

	ApplyDisplayValues();
}

void URuntimeGameplaySettingsFloatArrayElementWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Editable_Value)
	{
		Editable_Value->OnTextChanged.AddDynamic(this, &URuntimeGameplaySettingsFloatArrayElementWidget::HandleValueChanged);
		Editable_Value->OnTextCommitted.AddDynamic(this, &URuntimeGameplaySettingsFloatArrayElementWidget::HandleValueCommitted);
	}

	if (!bHasInitializedValue)
	{
		CurrentValue = DefaultValue;
		bHasInitializedValue = true;
	}

	ApplyDisplayValues();
}

void URuntimeGameplaySettingsFloatArrayElementWidget::BuildDefaultWidgetTree()
{
	using namespace RuntimeGameplaySettingsElementWidgetHelpers;

	if (!WidgetTree)
	{
		return;
	}

	UHorizontalBox* RootBox = ConstructWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_26"));
	Editable_Value = ConstructWidget<UEditableTextBox>(WidgetTree, TEXT("Editable_Value"));
	DeleteButt = CreateTextButton(
		WidgetTree,
		TEXT("DeleteButt"),
		TEXT("TextBlock_51"),
		INVTEXT("-"),
		DeleteNormalTint(),
		DeletePressedTint(),
		35.0f);
	ApplyEditableTextBoxStyle(Editable_Value);
	SetButtonContentPadding(DeleteButt, FMargin(0.0f, -7.0f, 0.0f, -5.0f));

	if (!RootBox)
	{
		return;
	}

	AddToHorizontalBox(RootBox, WrapInSizeBox(WidgetTree, TEXT("SizeBox_1"), Editable_Value, ValueWidth));
	AddToHorizontalBox(
		RootBox,
		DeleteButt,
		FMargin(10.0f, 0.0f, 0.0f, 0.0f),
		HAlign_Center,
		VAlign_Center);

	WidgetTree->RootWidget = RootBox;
}

void URuntimeGameplaySettingsFloatArrayElementWidget::HandleValueCommitted(const FText& InText, ETextCommit::Type CommitMethod)
{
	(void)CommitMethod;

	if (bIsSynchronizing)
	{
		return;
	}

	float ParsedValue = 0.0f;
	if (TryParseValue(InText, ParsedValue))
	{
		CurrentValue = ParsedValue;
		bHasInitializedValue = true;
	}

	ApplyDisplayValues();
	NotifyElementValueChanged();
}

void URuntimeGameplaySettingsFloatArrayElementWidget::HandleValueChanged(const FText& InText)
{
	(void)InText;

	if (bIsSynchronizing)
	{
		return;
	}

	NotifyElementValueChanged();
}

void URuntimeGameplaySettingsFloatArrayElementWidget::ApplyDisplayValues()
{
	TGuardValue<bool> SynchronizationGuard(bIsSynchronizing, true);

	if (Editable_Value)
	{
		Editable_Value->SetText(FText::FromString(FormatFloat(CurrentValue)));
	}
}

bool URuntimeGameplaySettingsFloatArrayElementWidget::TryParseValue(const FText& InText, float& OutValue) const
{
	FString ValueText = InText.ToString();
	ValueText.TrimStartAndEndInline();
	return FDefaultValueHelper::ParseFloat(ValueText, OutValue);
}

FString URuntimeGameplaySettingsFloatArrayElementWidget::FormatFloat(float InValue) const
{
	FString FormattedValue = FString::Printf(TEXT("%.3f"), InValue);

	while (FormattedValue.Contains(TEXT(".")) && FormattedValue.EndsWith(TEXT("0")))
	{
		FormattedValue.LeftChopInline(1, EAllowShrinking::No);
	}

	if (FormattedValue.EndsWith(TEXT(".")))
	{
		FormattedValue.LeftChopInline(1, EAllowShrinking::No);
	}

	return FormattedValue;
}
