#include "UI/Elements/RuntimeGameplaySettingsFloatArrayWidget.h"

#include "Settings/RuntimeGameplaySettingsProjectSettings.h"
#include "UI/Elements/RuntimeGameplaySettingsFloatArrayElementWidget.h"

void URuntimeGameplaySettingsFloatArrayWidget::SetFloatValues(const TArray<float>& InValues)
{
	CurrentValues = InValues;
	bHasInitializedValues = true;
	RebuildArrayElements();
}

void URuntimeGameplaySettingsFloatArrayWidget::SetDefaultFloatValues(const TArray<float>& InValues)
{
	RuntimeDefaultValues = InValues;
	bHasRuntimeDefaultValues = true;
	UpdateResetButtonState();
}

TArray<float> URuntimeGameplaySettingsFloatArrayWidget::GetFloatValues() const
{
	if (ElementWidgets.Num() == 0)
	{
		return CurrentValues;
	}

	TArray<float> Values;
	Values.Reserve(ElementWidgets.Num());

	for (const URuntimeGameplaySettingsArrayElementWidgetBase* ElementWidget : ElementWidgets)
	{
		if (const URuntimeGameplaySettingsFloatArrayElementWidget* FloatElementWidget =
			Cast<URuntimeGameplaySettingsFloatArrayElementWidget>(ElementWidget))
		{
			Values.Add(FloatElementWidget->GetFloatValue());
		}
	}

	return Values;
}

void URuntimeGameplaySettingsFloatArrayWidget::NativePreConstruct()
{
	if (!bHasInitializedValues)
	{
		CurrentValues = DefaultValues;
	}
	if (!bHasRuntimeDefaultValues)
	{
		RuntimeDefaultValues = DefaultValues;
	}

	Super::NativePreConstruct();
}

void URuntimeGameplaySettingsFloatArrayWidget::NativeOnInitialized()
{
	if (!bHasInitializedValues)
	{
		CurrentValues = DefaultValues;
		bHasInitializedValues = true;
	}
	if (!bHasRuntimeDefaultValues)
	{
		RuntimeDefaultValues = DefaultValues;
	}

	Super::NativeOnInitialized();
}

int32 URuntimeGameplaySettingsFloatArrayWidget::GetArrayValueCount() const
{
	return CurrentValues.Num();
}

void URuntimeGameplaySettingsFloatArrayWidget::AppendDefaultArrayValue()
{
	CurrentValues.Add(NewElementDefaultValue);
	bHasInitializedValues = true;
}

void URuntimeGameplaySettingsFloatArrayWidget::RemoveArrayValueAt(int32 ElementIndex)
{
	if (CurrentValues.IsValidIndex(ElementIndex))
	{
		CurrentValues.RemoveAt(ElementIndex);
		bHasInitializedValues = true;
	}
}

void URuntimeGameplaySettingsFloatArrayWidget::ResetArrayValuesToDefault()
{
	CurrentValues = bHasRuntimeDefaultValues ? RuntimeDefaultValues : DefaultValues;
	bHasInitializedValues = true;
}

bool URuntimeGameplaySettingsFloatArrayWidget::IsArrayValueAtDefault() const
{
	const TArray<float> Values = GetFloatValues();
	const TArray<float>& DefaultFloatValues = bHasRuntimeDefaultValues ? RuntimeDefaultValues : DefaultValues;
	if (Values.Num() != DefaultFloatValues.Num())
	{
		return false;
	}

	for (int32 ValueIndex = 0; ValueIndex < Values.Num(); ++ValueIndex)
	{
		if (!FMath::IsNearlyEqual(Values[ValueIndex], DefaultFloatValues[ValueIndex], 0.001f))
		{
			return false;
		}
	}

	return true;
}

void URuntimeGameplaySettingsFloatArrayWidget::SynchronizeArrayValuesFromElementWidgets()
{
	if (ElementWidgets.Num() > 0)
	{
		CurrentValues = GetFloatValues();
		bHasInitializedValues = true;
	}
}

void URuntimeGameplaySettingsFloatArrayWidget::InitializeElementWidget(
	URuntimeGameplaySettingsArrayElementWidgetBase* ElementWidget,
	int32 ElementIndex)
{
	URuntimeGameplaySettingsFloatArrayElementWidget* FloatElementWidget = Cast<URuntimeGameplaySettingsFloatArrayElementWidget>(ElementWidget);
	if (FloatElementWidget && CurrentValues.IsValidIndex(ElementIndex))
	{
		FloatElementWidget->SetFloatValue(CurrentValues[ElementIndex]);
	}
}

TSubclassOf<URuntimeGameplaySettingsArrayElementWidgetBase> URuntimeGameplaySettingsFloatArrayWidget::GetResolvedElementWidgetClass() const
{
	if (ElementWidgetClass && ElementWidgetClass->IsChildOf(URuntimeGameplaySettingsFloatArrayElementWidget::StaticClass()))
	{
		return ElementWidgetClass;
	}

	if (const URuntimeGameplaySettingsProjectSettings* Settings =
		GetDefault<URuntimeGameplaySettingsProjectSettings>())
	{
		if (UClass* ConfiguredClass = Settings->FloatArrayElementWidgetClass.LoadSynchronous())
		{
			if (ConfiguredClass->IsChildOf(URuntimeGameplaySettingsFloatArrayElementWidget::StaticClass()))
			{
				return ConfiguredClass;
			}
		}
	}

	return URuntimeGameplaySettingsFloatArrayElementWidget::StaticClass();
}
