#include "UI/Elements/RuntimeGameplaySettingsEnumWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/ComboBoxString.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Templates/UnrealTemplate.h"
#include "UI/Elements/RuntimeGameplaySettingsElementWidgetHelpers.h"
#include "UObject/UObjectIterator.h"

URuntimeGameplaySettingsEnumWidget::URuntimeGameplaySettingsEnumWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void URuntimeGameplaySettingsEnumWidget::SetOptions(const TArray<FRuntimeGameplaySettingsEnumOption>& InOptions)
{
	EnumClassPath.Reset();
	Options = InOptions;
	RebuildOptions();
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsEnumWidget::SetOptionsFromEnum(const UEnum* InEnum, bool bSkipHiddenOptions)
{
	EnumClassPath = InEnum ? InEnum->GetPathName() : FString();
	bSkipHiddenEnumOptions = bSkipHiddenOptions;

	RefreshOptionsFromEnumClass();
	RebuildOptions();
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsEnumWidget::SetSelectedValue(int32 InValue)
{
	CurrentValue = InValue;
	bHasInitializedValue = true;
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsEnumWidget::SetDefaultSelectedValue(int32 InValue)
{
	RuntimeDefaultValue = InValue;
	bHasRuntimeDefaultValue = true;
	UpdateResetButtonState();
}

int32 URuntimeGameplaySettingsEnumWidget::GetSelectedValue() const
{
	if (Combo_Options)
	{
		if (const FRuntimeGameplaySettingsEnumOption* SelectedOption = FindOptionByDisplayName(Combo_Options->GetSelectedOption()))
		{
			return SelectedOption->Value;
		}
	}

	return CurrentValue;
}

TArray<FString> URuntimeGameplaySettingsEnumWidget::GetEnumClassPathOptions() const
{
	TArray<FString> EnumPaths;

	const auto AddEnumPathIfValid = [&EnumPaths](const FString& InEnumPath)
	{
		const FString SanitizedEnumPath = InEnumPath.TrimStartAndEnd();
		if (!SanitizedEnumPath.IsEmpty())
		{
			EnumPaths.AddUnique(SanitizedEnumPath);
		}
	};

	AddEnumPathIfValid(EnumClassPath);
	for (const FString& AdditionalEnumClassPath : AdditionalEnumClassPaths)
	{
		AddEnumPathIfValid(AdditionalEnumClassPath);
	}

	for (TObjectIterator<UEnum> EnumIt; EnumIt; ++EnumIt)
	{
		const UEnum* EnumClass = *EnumIt;
		if (!EnumClass || EnumClass->HasAnyFlags(RF_Transient))
		{
			continue;
		}

		const FString EnumPath = EnumClass->GetPathName();
		AddEnumPathIfValid(EnumPath);
	}

	EnumPaths.Sort();
	return EnumPaths;
}

void URuntimeGameplaySettingsEnumWidget::NativePreConstruct()
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

	RefreshOptionsFromEnumClass();
	RebuildOptions();
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsEnumWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Combo_Options)
	{
		Combo_Options->OnSelectionChanged.AddDynamic(this, &URuntimeGameplaySettingsEnumWidget::HandleSelectionChanged);
	}
	CurrentValue = DefaultValue;
	bHasInitializedValue = true;
	if (!bHasRuntimeDefaultValue)
	{
		RuntimeDefaultValue = DefaultValue;
	}

	RefreshOptionsFromEnumClass();
	RebuildOptions();
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsEnumWidget::ResetValueToDefault()
{
	SetSelectedValue(GetDefaultSelectedValue());
}

bool URuntimeGameplaySettingsEnumWidget::IsValueAtDefault() const
{
	return GetSelectedValue() == GetDefaultSelectedValue();
}

void URuntimeGameplaySettingsEnumWidget::BuildDefaultWidgetTree()
{
	using namespace RuntimeGameplaySettingsElementWidgetHelpers;

	if (!WidgetTree)
	{
		return;
	}

	UHorizontalBox* RootBox = ConstructWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_2"));
	Txt_Name = CreateTextBlock(WidgetTree, TEXT("Txt_Name"), INVTEXT("Param Name"), LabelFontSize);
	Combo_Options = ConstructWidget<UComboBoxString>(WidgetTree, TEXT("Combo_Options"));
	ResetButtonWidget = CreateResetButtonWidget(WidgetTree);

	if (!RootBox)
	{
		return;
	}

	if (UHorizontalBoxSlot* NameSlot = RootBox->AddChildToHorizontalBox(WrapLabelInSizeBox(WidgetTree, Txt_Name)))
	{
		NameSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		NameSlot->SetPadding(FMargin(0.0f));
		NameSlot->SetHorizontalAlignment(HAlign_Left);
		NameSlot->SetVerticalAlignment(VAlign_Center);
	}
	AddToHorizontalBox(
		RootBox,
		WrapInSizeBox(WidgetTree, TEXT("SizeBox_1"), Combo_Options, ValueWidth),
		FMargin(0.0f),
		HAlign_Right);
	AddToHorizontalBox(RootBox, ResetButtonWidget, FMargin(10.0f, 0.0f, 0.0f, 0.0f), HAlign_Right);

	WidgetTree->RootWidget = RootBox;
}

void URuntimeGameplaySettingsEnumWidget::HandleSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	(void)SelectionType;

	if (bIsSynchronizing)
	{
		return;
	}

	if (const FRuntimeGameplaySettingsEnumOption* SelectedOption = FindOptionByDisplayName(SelectedItem))
	{
		CurrentValue = SelectedOption->Value;
		bHasInitializedValue = true;
	}

	ApplyDisplayValues();
}

void URuntimeGameplaySettingsEnumWidget::ApplyDisplayValues()
{
	TGuardValue<bool> SynchronizationGuard(bIsSynchronizing, true);

	if (Combo_Options)
	{
		if (const FRuntimeGameplaySettingsEnumOption* SelectedOption = FindOptionByValue(CurrentValue))
		{
			Combo_Options->SetSelectedOption(SelectedOption->DisplayName.ToString());
		}
		else if (Options.Num() > 0)
		{
			CurrentValue = Options[0].Value;
			Combo_Options->SetSelectedOption(Options[0].DisplayName.ToString());
		}
		else
		{
			Combo_Options->ClearSelection();
		}
	}

	Super::ApplyDisplayValues();
}

const UEnum* URuntimeGameplaySettingsEnumWidget::ResolveEnumClass() const
{
	const FString SanitizedEnumClassPath = EnumClassPath.TrimStartAndEnd();
	if (SanitizedEnumClassPath.IsEmpty())
	{
		return nullptr;
	}

	if (const UEnum* ExistingEnum = FindObject<UEnum>(nullptr, *SanitizedEnumClassPath))
	{
		return ExistingEnum;
	}

	return LoadObject<UEnum>(nullptr, *SanitizedEnumClassPath);
}

void URuntimeGameplaySettingsEnumWidget::RefreshOptionsFromEnumClass()
{
	const UEnum* ResolvedEnumClass = ResolveEnumClass();
	if (!ResolvedEnumClass)
	{
		return;
	}

	Options.Reset();

	const int32 EnumOptionCount = ResolvedEnumClass->NumEnums();
	for (int32 EnumOptionIndex = 0; EnumOptionIndex < EnumOptionCount; ++EnumOptionIndex)
	{
		if (bSkipHiddenEnumOptions && ResolvedEnumClass->HasMetaData(TEXT("Hidden"), EnumOptionIndex))
		{
			continue;
		}

		const FString OptionName = ResolvedEnumClass->GetNameStringByIndex(EnumOptionIndex);
		if (bSkipHiddenEnumOptions && OptionName.EndsWith(TEXT("_MAX")))
		{
			continue;
		}

		FRuntimeGameplaySettingsEnumOption Option;
		Option.DisplayName = ResolvedEnumClass->GetDisplayNameTextByIndex(EnumOptionIndex);
		if (Option.DisplayName.IsEmpty())
		{
			Option.DisplayName = FText::FromString(OptionName);
		}
		Option.Value = static_cast<int32>(ResolvedEnumClass->GetValueByIndex(EnumOptionIndex));
		Options.Add(Option);
	}
}

void URuntimeGameplaySettingsEnumWidget::RebuildOptions()
{
	if (!Combo_Options)
	{
		return;
	}

	TGuardValue<bool> SynchronizationGuard(bIsSynchronizing, true);

	Combo_Options->ClearOptions();
	for (const FRuntimeGameplaySettingsEnumOption& Option : Options)
	{
		Combo_Options->AddOption(Option.DisplayName.ToString());
	}
}

const FRuntimeGameplaySettingsEnumOption* URuntimeGameplaySettingsEnumWidget::FindOptionByValue(int32 InValue) const
{
	return Options.FindByPredicate([InValue](const FRuntimeGameplaySettingsEnumOption& Option)
	{
		return Option.Value == InValue;
	});
}

const FRuntimeGameplaySettingsEnumOption* URuntimeGameplaySettingsEnumWidget::FindOptionByDisplayName(const FString& InDisplayName) const
{
	return Options.FindByPredicate([&InDisplayName](const FRuntimeGameplaySettingsEnumOption& Option)
	{
		return Option.DisplayName.ToString().Equals(InDisplayName, ESearchCase::CaseSensitive);
	});
}

int32 URuntimeGameplaySettingsEnumWidget::GetDefaultSelectedValue() const
{
	return bHasRuntimeDefaultValue ? RuntimeDefaultValue : DefaultValue;
}
