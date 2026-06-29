#include "UI/Elements/RuntimeGameplaySettingsParameterWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/UnrealTemplate.h"
#include "UI/Elements/RuntimeGameplaySettingsElementWidgetHelpers.h"

namespace
{
constexpr float RuntimeGameplaySettingsParameterResetTolerance = 0.001f;
}

void URuntimeGameplaySettingsParameterWidget::SetParameterValue(float InValue)
{
	CurrentValue = InValue;
	bHasInitializedValue = true;
	SyncWidgetsToCurrentValue();
}

void URuntimeGameplaySettingsParameterWidget::SetDefaultParameterValue(float InValue)
{
	RuntimeDefaultValue = InValue;
	bHasRuntimeDefaultValue = true;
	UpdateResetButtonState();
}

float URuntimeGameplaySettingsParameterWidget::GetParameterValue() const
{
	return CurrentValue;
}

float URuntimeGameplaySettingsParameterWidget::GetEditableParameterValue() const
{
	if (!Editable_Param)
	{
		return CurrentValue;
	}

	float ParsedValue = CurrentValue;
	if (TryParseFloatText(Editable_Param->GetText(), ParsedValue))
	{
		return ParsedValue;
	}

	return CurrentValue;
}

void URuntimeGameplaySettingsParameterWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!bHasInitializedValue)
	{
		CurrentValue = DefaultValue;
	}
	if (!bHasRuntimeDefaultValue)
	{
		RuntimeDefaultValue = DefaultValue;
	}

	ApplyParameterMetadata();
	SyncWidgetsToCurrentValue();
}

void URuntimeGameplaySettingsParameterWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Slider_Param)
	{
		Slider_Param->OnValueChanged.AddDynamic(this, &URuntimeGameplaySettingsParameterWidget::HandleSliderValueChanged);
	}

	if (Editable_Param)
	{
		Editable_Param->OnTextChanged.AddDynamic(this, &URuntimeGameplaySettingsParameterWidget::HandleEditableTextChanged);
		Editable_Param->OnTextCommitted.AddDynamic(this, &URuntimeGameplaySettingsParameterWidget::HandleEditableTextCommitted);
	}
	CurrentValue = DefaultValue;
	bHasInitializedValue = true;
	if (!bHasRuntimeDefaultValue)
	{
		RuntimeDefaultValue = DefaultValue;
	}

	ApplyParameterMetadata();
	SyncWidgetsToCurrentValue();
}

void URuntimeGameplaySettingsParameterWidget::ApplyDisplayValues()
{
	ApplyParameterMetadata();
	SyncWidgetsToCurrentValue();
}

void URuntimeGameplaySettingsParameterWidget::ResetValueToDefault()
{
	SetParameterValue(GetDefaultParameterValue());
}

bool URuntimeGameplaySettingsParameterWidget::IsValueAtDefault() const
{
	return FMath::IsNearlyEqual(
		GetEditableParameterValue(),
		GetDefaultParameterValue(),
		RuntimeGameplaySettingsParameterResetTolerance);
}

void URuntimeGameplaySettingsParameterWidget::BuildDefaultWidgetTree()
{
	using namespace RuntimeGameplaySettingsElementWidgetHelpers;

	if (!WidgetTree)
	{
		return;
	}

	UHorizontalBox* RootBox = ConstructWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_2"));
	Txt_Name = CreateTextBlock(WidgetTree, TEXT("Txt_Name"), INVTEXT("Param Name"), LabelFontSize);
	Slider_Param = ConstructWidget<USlider>(WidgetTree, TEXT("Slider_Param"));
	Editable_Param = ConstructWidget<UEditableTextBox>(WidgetTree, TEXT("Editable_Param"));
	ResetButtonWidget = CreateResetButtonWidget(WidgetTree);
	ApplySliderThumbSize(Slider_Param, FVector2D(15.0f, 50.0f));
	ApplyEditableTextBoxStyle(Editable_Param);

	if (!RootBox)
	{
		return;
	}

	AddToHorizontalBox(RootBox, WrapLabelInSizeBox(WidgetTree, Txt_Name), FMargin(0.0f), HAlign_Left, VAlign_Center);
	if (UHorizontalBoxSlot* SliderSlot = RootBox->AddChildToHorizontalBox(
		WrapInSizeBox(WidgetTree, TEXT("SizeBox_100"), Slider_Param, SliderWidth)))
	{
		SliderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		SliderSlot->SetPadding(FMargin(30.0f, 0.0f, 30.0f, 0.0f));
		SliderSlot->SetHorizontalAlignment(HAlign_Center);
		SliderSlot->SetVerticalAlignment(VAlign_Center);
	}
	AddToHorizontalBox(RootBox, WrapInSizeBox(WidgetTree, TEXT("SizeBox_1"), Editable_Param, ValueWidth), FMargin(0.0f));
	AddToHorizontalBox(RootBox, ResetButtonWidget, FMargin(10.0f, 0.0f, 0.0f, 0.0f), HAlign_Right);

	WidgetTree->RootWidget = RootBox;
}

void URuntimeGameplaySettingsParameterWidget::HandleSliderValueChanged(float InValue)
{
	if (bIsSynchronizing)
	{
		return;
	}

	CurrentValue = SanitizeValue(InValue);
	bHasInitializedValue = true;
	SyncWidgetsToCurrentValue();
}

void URuntimeGameplaySettingsParameterWidget::HandleEditableTextCommitted(const FText& InText, ETextCommit::Type CommitMethod)
{
	(void)CommitMethod;

	if (bIsSynchronizing)
	{
		return;
	}

	float ParsedValue = 0.0f;
	if (TryParseFloatText(InText, ParsedValue))
	{
		CurrentValue = ParsedValue;
		bHasInitializedValue = true;
	}

	SyncWidgetsToCurrentValue();
}

void URuntimeGameplaySettingsParameterWidget::HandleEditableTextChanged(const FText& InText)
{
	(void)InText;

	if (bIsSynchronizing)
	{
		return;
	}

	UpdateResetButtonState();
}

void URuntimeGameplaySettingsParameterWidget::ApplyParameterMetadata()
{
	Super::ApplyDisplayValues();

	if (Slider_Param)
	{
		const float SafeMinValue = FMath::Min(MinValue, MaxValue);
		const float SafeMaxValue = FMath::Max(MinValue, MaxValue);

		Slider_Param->SetMinValue(SafeMinValue);
		Slider_Param->SetMaxValue(SafeMaxValue);
		Slider_Param->SetStepSize(FMath::Max(StepSize, KINDA_SMALL_NUMBER));
	}
}

void URuntimeGameplaySettingsParameterWidget::SyncWidgetsToCurrentValue()
{
	TGuardValue<bool> SynchronizationGuard(bIsSynchronizing, true);

	if (Slider_Param)
	{
		Slider_Param->SetValue(CurrentValue);
	}

	if (Editable_Param)
	{
		Editable_Param->SetText(FormatFloatAsText(CurrentValue));
	}

	UpdateResetButtonState();
}

float URuntimeGameplaySettingsParameterWidget::GetDefaultParameterValue() const
{
	return bHasRuntimeDefaultValue ? RuntimeDefaultValue : DefaultValue;
}

float URuntimeGameplaySettingsParameterWidget::SanitizeValue(float InValue) const
{
	const float SafeMinValue = FMath::Min(MinValue, MaxValue);
	const float SafeMaxValue = FMath::Max(MinValue, MaxValue);
	float SanitizedValue = FMath::Clamp(InValue, SafeMinValue, SafeMaxValue);

	if (StepSize > KINDA_SMALL_NUMBER)
	{
		const float StepsFromMin = FMath::RoundToFloat((SanitizedValue - SafeMinValue) / StepSize);
		SanitizedValue = SafeMinValue + StepsFromMin * StepSize;
		SanitizedValue = FMath::Clamp(SanitizedValue, SafeMinValue, SafeMaxValue);
	}

	return SanitizedValue;
}
