#include "UI/Elements/RuntimeGameplaySettingsCurveWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/TextBlock.h"
#include "Misc/DefaultValueHelper.h"
#include "Templates/UnrealTemplate.h"
#include "UI/Elements/RuntimeGameplaySettingsElementWidgetHelpers.h"

namespace
{
constexpr float RuntimeGameplaySettingsCurveResetTolerance = 0.001f;
}

void URuntimeGameplaySettingsCurveWidget::SetCurveValue(const FRuntimeFloatCurve& InCurveValue)
{
	CurrentCurve = InCurveValue;
	bHasInitializedCurve = true;
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsCurveWidget::SetDefaultCurveValue(const FRuntimeFloatCurve& InCurveValue)
{
	RuntimeDefaultCurve = InCurveValue;
	bHasRuntimeDefaultCurve = true;
	UpdateResetButtonState();
}

FRuntimeFloatCurve URuntimeGameplaySettingsCurveWidget::GetCurveValue() const
{
	if (!Editable_Curve)
	{
		return CurrentCurve;
	}

	FRuntimeFloatCurve ParsedCurve;
	if (TryParseCurve(Editable_Curve->GetText(), ParsedCurve))
	{
		return ParsedCurve;
	}

	return CurrentCurve;
}

void URuntimeGameplaySettingsCurveWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!bHasInitializedCurve)
	{
		CurrentCurve = DefaultCurve;
	}
	if (!bHasRuntimeDefaultCurve)
	{
		RuntimeDefaultCurve = DefaultCurve;
	}

	ApplyDisplayValues();
}

void URuntimeGameplaySettingsCurveWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Editable_Curve)
	{
		Editable_Curve->OnTextChanged.AddDynamic(this, &URuntimeGameplaySettingsCurveWidget::HandleCurveTextChanged);
		Editable_Curve->OnTextCommitted.AddDynamic(this, &URuntimeGameplaySettingsCurveWidget::HandleCurveCommitted);
	}
	CurrentCurve = DefaultCurve;
	bHasInitializedCurve = true;
	if (!bHasRuntimeDefaultCurve)
	{
		RuntimeDefaultCurve = DefaultCurve;
	}
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsCurveWidget::ResetValueToDefault()
{
	SetCurveValue(GetDefaultCurveValue());
}

bool URuntimeGameplaySettingsCurveWidget::IsValueAtDefault() const
{
	return AreCurvesEquivalent(GetCurveValue(), GetDefaultCurveValue());
}

void URuntimeGameplaySettingsCurveWidget::BuildDefaultWidgetTree()
{
	using namespace RuntimeGameplaySettingsElementWidgetHelpers;

	if (!WidgetTree)
	{
		return;
	}

	UHorizontalBox* RootBox = ConstructWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_2"));
	Txt_Name = CreateTextBlock(WidgetTree, TEXT("Txt_Name"), INVTEXT("Param Name"), LabelFontSize);
	Editable_Curve = ConstructWidget<UMultiLineEditableTextBox>(WidgetTree, TEXT("Editable_Curve"));
	ResetButtonWidget = CreateResetButtonWidget(WidgetTree);

	if (!RootBox)
	{
		return;
	}

	AddToHorizontalBox(RootBox, WrapLabelInSizeBox(WidgetTree, Txt_Name), FMargin(0.0f), HAlign_Left, VAlign_Center);
	AddToHorizontalBox(
		RootBox,
		WrapInSizeBox(WidgetTree, TEXT("SizeBox_1"), Editable_Curve, 500.0f),
		FMargin(10.0f, 0.0f, 0.0f, 0.0f),
		HAlign_Left);
	AddToHorizontalBox(RootBox, ResetButtonWidget, FMargin(10.0f, 0.0f, 0.0f, 0.0f), HAlign_Right);

	WidgetTree->RootWidget = RootBox;
}

void URuntimeGameplaySettingsCurveWidget::HandleCurveCommitted(const FText& InText, ETextCommit::Type CommitMethod)
{
	(void)CommitMethod;

	if (bIsSynchronizing)
	{
		return;
	}

	FRuntimeFloatCurve ParsedCurve;
	if (TryParseCurve(InText, ParsedCurve))
	{
		CurrentCurve = ParsedCurve;
		bHasInitializedCurve = true;
	}

	ApplyDisplayValues();
}

void URuntimeGameplaySettingsCurveWidget::HandleCurveTextChanged(const FText& InText)
{
	(void)InText;

	if (bIsSynchronizing)
	{
		return;
	}

	UpdateResetButtonState();
}

void URuntimeGameplaySettingsCurveWidget::ApplyDisplayValues()
{
	TGuardValue<bool> SynchronizationGuard(bIsSynchronizing, true);

	if (Editable_Curve)
	{
		Editable_Curve->SetText(FormatCurveAsText(CurrentCurve));
	}

	Super::ApplyDisplayValues();
}

const FRuntimeFloatCurve& URuntimeGameplaySettingsCurveWidget::GetDefaultCurveValue() const
{
	return bHasRuntimeDefaultCurve ? RuntimeDefaultCurve : DefaultCurve;
}

bool URuntimeGameplaySettingsCurveWidget::AreCurvesEquivalent(
	const FRuntimeFloatCurve& FirstCurve,
	const FRuntimeFloatCurve& SecondCurve) const
{
	const FRichCurve* FirstRichCurve = FirstCurve.GetRichCurveConst();
	const FRichCurve* SecondRichCurve = SecondCurve.GetRichCurveConst();
	if (!FirstRichCurve || !SecondRichCurve)
	{
		return FirstRichCurve == SecondRichCurve;
	}

	TArray<TPair<float, float>> FirstKeys;
	TArray<TPair<float, float>> SecondKeys;
	for (auto KeyIt = FirstRichCurve->GetKeyIterator(); KeyIt; ++KeyIt)
	{
		FirstKeys.Emplace(KeyIt->Time, KeyIt->Value);
	}
	for (auto KeyIt = SecondRichCurve->GetKeyIterator(); KeyIt; ++KeyIt)
	{
		SecondKeys.Emplace(KeyIt->Time, KeyIt->Value);
	}

	if (FirstKeys.Num() != SecondKeys.Num())
	{
		return false;
	}

	for (int32 KeyIndex = 0; KeyIndex < FirstKeys.Num(); ++KeyIndex)
	{
		if (!FMath::IsNearlyEqual(FirstKeys[KeyIndex].Key, SecondKeys[KeyIndex].Key, RuntimeGameplaySettingsCurveResetTolerance)
			|| !FMath::IsNearlyEqual(FirstKeys[KeyIndex].Value, SecondKeys[KeyIndex].Value, RuntimeGameplaySettingsCurveResetTolerance))
		{
			return false;
		}
	}

	return true;
}

FText URuntimeGameplaySettingsCurveWidget::FormatCurveAsText(const FRuntimeFloatCurve& InCurveValue) const
{
	const FRichCurve* RichCurve = InCurveValue.GetRichCurveConst();
	if (!RichCurve)
	{
		return FText::GetEmpty();
	}

	TArray<FString> FormattedKeys;
	for (auto KeyIt = RichCurve->GetKeyIterator(); KeyIt; ++KeyIt)
	{
		FormattedKeys.Add(FString::Printf(
			TEXT("%s=%s"),
			*FormatFloatAsString(KeyIt->Time),
			*FormatFloatAsString(KeyIt->Value)));
	}

	return FText::FromString(FString::Join(FormattedKeys, TEXT("\n")));
}

bool URuntimeGameplaySettingsCurveWidget::TryParseCurve(const FText& InText, FRuntimeFloatCurve& OutCurveValue) const
{
	FString NormalizedText = InText.ToString();
	NormalizedText.ReplaceInline(TEXT("\r"), TEXT("\n"));
	NormalizedText.ReplaceInline(TEXT(";"), TEXT("\n"));
	NormalizedText.ReplaceInline(TEXT(","), TEXT("\n"));

	TArray<FString> Tokens;
	NormalizedText.ParseIntoArray(Tokens, TEXT("\n"), true);

	FRichCurve* RichCurve = OutCurveValue.GetRichCurve();
	if (!RichCurve)
	{
		return false;
	}

	RichCurve->Reset();

	for (FString Token : Tokens)
	{
		Token.TrimStartAndEndInline();
		if (Token.IsEmpty())
		{
			continue;
		}

		float Time = 0.0f;
		float Value = 0.0f;
		if (!TryParseCurvePoint(Token, Time, Value))
		{
			return false;
		}

		RichCurve->AddKey(Time, Value);
	}

	return true;
}

bool URuntimeGameplaySettingsCurveWidget::TryParseCurvePoint(const FString& InText, float& OutTime, float& OutValue) const
{
	FString PairText = InText;
	PairText.ReplaceInline(TEXT("="), TEXT(" "));
	PairText.ReplaceInline(TEXT(":"), TEXT(" "));
	PairText.TrimStartAndEndInline();

	TArray<FString> Parts;
	PairText.ParseIntoArrayWS(Parts);
	if (Parts.Num() != 2)
	{
		return false;
	}

	return FDefaultValueHelper::ParseFloat(Parts[0], OutTime)
		&& FDefaultValueHelper::ParseFloat(Parts[1], OutValue);
}
