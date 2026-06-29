#include "UI/Elements/RuntimeGameplaySettingsValueWidgetBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Containers/AllowShrinking.h"
#include "Misc/DefaultValueHelper.h"
#include "UI/Elements/RuntimeGameplaySettingsResetButtonWidget.h"

void URuntimeGameplaySettingsValueWidgetBase::SetParameterName(const FText& InParameterName)
{
	EnsureDefaultWidgetTree();
	ParameterName = InParameterName;
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsValueWidgetBase::ResetToDefaultValue()
{
	EnsureDefaultWidgetTree();
	ResetValueToDefault();
}

void URuntimeGameplaySettingsValueWidgetBase::NativePreConstruct()
{
	EnsureDefaultWidgetTree();
	Super::NativePreConstruct();
}

void URuntimeGameplaySettingsValueWidgetBase::NativeOnInitialized()
{
	EnsureDefaultWidgetTree();
	Super::NativeOnInitialized();

	if (ResetButt)
	{
		ResetButt->OnReleased.AddDynamic(this, &URuntimeGameplaySettingsValueWidgetBase::HandleResetReleased);
	}

	if (ResetButtonWidget)
	{
		ResetButtonWidget->OnRuntimeGameplaySettingsResetButtonReleased.AddDynamic(
			this,
			&URuntimeGameplaySettingsValueWidgetBase::HandleResetWidgetReleased);
	}
}

void URuntimeGameplaySettingsValueWidgetBase::ApplyDisplayValues()
{
	if (Txt_Name)
	{
		Txt_Name->SetText(ParameterName);
	}

	UpdateResetButtonState();
}

void URuntimeGameplaySettingsValueWidgetBase::ResetValueToDefault()
{
}

bool URuntimeGameplaySettingsValueWidgetBase::IsValueAtDefault() const
{
	return true;
}

void URuntimeGameplaySettingsValueWidgetBase::UpdateResetButtonState()
{
	const bool bIsResetEnabled = !IsValueAtDefault();
	if (ResetButt)
	{
		ResetButt->SetIsEnabled(bIsResetEnabled);
	}

	if (ResetButtonWidget)
	{
		ResetButtonWidget->SetResetEnabled(bIsResetEnabled);
	}
}

FText URuntimeGameplaySettingsValueWidgetBase::FormatFloatAsText(float InValue)
{
	return FText::FromString(FormatFloatAsString(InValue));
}

FString URuntimeGameplaySettingsValueWidgetBase::FormatFloatAsString(float InValue)
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

bool URuntimeGameplaySettingsValueWidgetBase::TryParseFloatText(const FText& InText, float& OutValue)
{
	return FDefaultValueHelper::ParseFloat(InText.ToString(), OutValue);
}

void URuntimeGameplaySettingsValueWidgetBase::HandleResetReleased()
{
	ResetToDefaultValue();
}

void URuntimeGameplaySettingsValueWidgetBase::HandleResetWidgetReleased()
{
	ResetToDefaultValue();
}

void URuntimeGameplaySettingsValueWidgetBase::BuildDefaultWidgetTree()
{
}

void URuntimeGameplaySettingsValueWidgetBase::EnsureDefaultWidgetTree()
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
