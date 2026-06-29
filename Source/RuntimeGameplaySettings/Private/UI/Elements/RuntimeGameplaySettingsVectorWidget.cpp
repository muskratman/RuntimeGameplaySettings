#include "UI/Elements/RuntimeGameplaySettingsVectorWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Templates/UnrealTemplate.h"
#include "UI/Elements/RuntimeGameplaySettingsElementWidgetHelpers.h"

namespace
{
constexpr float RuntimeGameplaySettingsVectorResetTolerance = 0.001f;
}

void URuntimeGameplaySettingsVectorWidget::SetVectorValue(const FVector& InVectorValue)
{
	X = InVectorValue.X;
	Y = InVectorValue.Y;
	Z = InVectorValue.Z;
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsVectorWidget::SetDefaultVectorValue(const FVector& InVectorValue)
{
	RuntimeDefaultValue = InVectorValue;
	bHasRuntimeDefaultValue = true;
	UpdateResetButtonState();
}

FVector URuntimeGameplaySettingsVectorWidget::GetVectorValue() const
{
	float ParsedX = X;
	float ParsedY = Y;
	float ParsedZ = Z;

	if (Editable_X)
	{
		TryParseFloatText(Editable_X->GetText(), ParsedX);
	}

	if (Editable_Y)
	{
		TryParseFloatText(Editable_Y->GetText(), ParsedY);
	}

	if (Editable_Z)
	{
		TryParseFloatText(Editable_Z->GetText(), ParsedZ);
	}

	return FVector(ParsedX, ParsedY, ParsedZ);
}

void URuntimeGameplaySettingsVectorWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	CacheDefaultVectorValueIfNeeded();
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsVectorWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Editable_X)
	{
		Editable_X->OnTextChanged.AddDynamic(this, &URuntimeGameplaySettingsVectorWidget::HandleVectorTextChanged);
		Editable_X->OnTextCommitted.AddDynamic(this, &URuntimeGameplaySettingsVectorWidget::HandleXTextCommitted);
	}

	if (Editable_Y)
	{
		Editable_Y->OnTextChanged.AddDynamic(this, &URuntimeGameplaySettingsVectorWidget::HandleVectorTextChanged);
		Editable_Y->OnTextCommitted.AddDynamic(this, &URuntimeGameplaySettingsVectorWidget::HandleYTextCommitted);
	}

	if (Editable_Z)
	{
		Editable_Z->OnTextChanged.AddDynamic(this, &URuntimeGameplaySettingsVectorWidget::HandleVectorTextChanged);
		Editable_Z->OnTextCommitted.AddDynamic(this, &URuntimeGameplaySettingsVectorWidget::HandleZTextCommitted);
	}
	CacheDefaultVectorValueIfNeeded();
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsVectorWidget::ResetValueToDefault()
{
	CacheDefaultVectorValueIfNeeded();
	SetVectorValue(RuntimeDefaultValue);
}

bool URuntimeGameplaySettingsVectorWidget::IsValueAtDefault() const
{
	const FVector CurrentVectorValue = GetVectorValue();
	const FVector DefaultVectorValue = GetDefaultVectorValue();
	return FMath::IsNearlyEqual(CurrentVectorValue.X, DefaultVectorValue.X, RuntimeGameplaySettingsVectorResetTolerance)
		&& FMath::IsNearlyEqual(CurrentVectorValue.Y, DefaultVectorValue.Y, RuntimeGameplaySettingsVectorResetTolerance)
		&& FMath::IsNearlyEqual(CurrentVectorValue.Z, DefaultVectorValue.Z, RuntimeGameplaySettingsVectorResetTolerance);
}

void URuntimeGameplaySettingsVectorWidget::BuildDefaultWidgetTree()
{
	using namespace RuntimeGameplaySettingsElementWidgetHelpers;

	if (!WidgetTree)
	{
		return;
	}

	UHorizontalBox* RootBox = ConstructWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_2"));
	UHorizontalBox* VectorBox = ConstructWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_0"));
	Txt_Name = CreateTextBlock(WidgetTree, TEXT("Txt_Name"), INVTEXT("Param Name"), LabelFontSize);
	Editable_X = ConstructWidget<UEditableTextBox>(WidgetTree, TEXT("Editable_X"));
	Editable_Y = ConstructWidget<UEditableTextBox>(WidgetTree, TEXT("Editable_Y"));
	Editable_Z = ConstructWidget<UEditableTextBox>(WidgetTree, TEXT("Editable_Z"));
	ResetButtonWidget = CreateResetButtonWidget(WidgetTree);
	ApplyEditableTextBoxStyle(Editable_X);
	ApplyEditableTextBoxStyle(Editable_Y);
	ApplyEditableTextBoxStyle(Editable_Z);
	if (Editable_X)
	{
		Editable_X->SetJustification(ETextJustify::Right);
	}
	if (Editable_Y)
	{
		Editable_Y->SetJustification(ETextJustify::Right);
	}
	if (Editable_Z)
	{
		Editable_Z->SetJustification(ETextJustify::Right);
	}

	if (!RootBox || !VectorBox)
	{
		return;
	}

	const auto BuildAxisBox = [this](
		const FName SizeBoxName,
		const FName AxisBoxName,
		const FName AxisTextName,
		const FText AxisText,
		UEditableTextBox* EditableWidget)
	{
		using namespace RuntimeGameplaySettingsElementWidgetHelpers;

		UHorizontalBox* AxisBox = ConstructWidget<UHorizontalBox>(WidgetTree, AxisBoxName);
		UTextBlock* AxisLabel = CreateTextBlock(WidgetTree, AxisTextName, AxisText, 24.0f);
		if (!AxisBox)
		{
			return static_cast<USizeBox*>(nullptr);
		}

		AddToHorizontalBox(AxisBox, AxisLabel);
		if (UHorizontalBoxSlot* EditableSlot = AxisBox->AddChildToHorizontalBox(EditableWidget))
		{
			EditableSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			EditableSlot->SetHorizontalAlignment(HAlign_Fill);
			EditableSlot->SetVerticalAlignment(VAlign_Center);
		}
		USizeBox* AxisSizeBox = WrapInSizeBox(WidgetTree, SizeBoxName, AxisBox, ValueWidth);
		if (AxisSizeBox)
		{
			AxisSizeBox->SetMinDesiredWidth(ValueWidth);
		}
		return AxisSizeBox;
	};

	AddToHorizontalBox(RootBox, WrapLabelInSizeBox(WidgetTree, Txt_Name), FMargin(0.0f), HAlign_Left, VAlign_Center);
	AddToHorizontalBox(
		VectorBox,
		BuildAxisBox(TEXT("SizeBox_X"), TEXT("HBox_X"), TEXT("Txt_X"), INVTEXT("X : "), Editable_X));
	AddToHorizontalBox(
		VectorBox,
		BuildAxisBox(TEXT("SizeBox_Y"), TEXT("HBox_Y"), TEXT("Txt_Y"), INVTEXT("Y : "), Editable_Y),
		FMargin(30.0f, 0.0f, 30.0f, 0.0f),
		HAlign_Left);
	AddToHorizontalBox(
		VectorBox,
		BuildAxisBox(TEXT("SizeBox_Z"), TEXT("HBox_Z"), TEXT("Txt_Z"), INVTEXT("Z : "), Editable_Z),
		FMargin(0.0f),
		HAlign_Left);
	if (UHorizontalBoxSlot* VectorSlot = RootBox->AddChildToHorizontalBox(VectorBox))
	{
		VectorSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		VectorSlot->SetPadding(FMargin(30.0f, 0.0f, 0.0f, 0.0f));
		VectorSlot->SetHorizontalAlignment(HAlign_Center);
		VectorSlot->SetVerticalAlignment(VAlign_Center);
	}
	AddToHorizontalBox(
		RootBox,
		ResetButtonWidget,
		FMargin(10.0f, 0.0f, 0.0f, 0.0f),
		HAlign_Right);

	WidgetTree->RootWidget = RootBox;
}

void URuntimeGameplaySettingsVectorWidget::HandleXTextCommitted(const FText& InText, ETextCommit::Type CommitMethod)
{
	(void)CommitMethod;

	if (bIsSynchronizing)
	{
		return;
	}

	float ParsedValue = X;
	if (TryParseFloatText(InText, ParsedValue))
	{
		X = ParsedValue;
	}

	ApplyDisplayValues();
}

void URuntimeGameplaySettingsVectorWidget::HandleYTextCommitted(const FText& InText, ETextCommit::Type CommitMethod)
{
	(void)CommitMethod;

	if (bIsSynchronizing)
	{
		return;
	}

	float ParsedValue = Y;
	if (TryParseFloatText(InText, ParsedValue))
	{
		Y = ParsedValue;
	}

	ApplyDisplayValues();
}

void URuntimeGameplaySettingsVectorWidget::HandleZTextCommitted(const FText& InText, ETextCommit::Type CommitMethod)
{
	(void)CommitMethod;

	if (bIsSynchronizing)
	{
		return;
	}

	float ParsedValue = Z;
	if (TryParseFloatText(InText, ParsedValue))
	{
		Z = ParsedValue;
	}

	ApplyDisplayValues();
}

void URuntimeGameplaySettingsVectorWidget::HandleVectorTextChanged(const FText& InText)
{
	(void)InText;

	if (bIsSynchronizing)
	{
		return;
	}

	UpdateResetButtonState();
}

void URuntimeGameplaySettingsVectorWidget::ApplyDisplayValues()
{
	TGuardValue<bool> SynchronizationGuard(bIsSynchronizing, true);

	if (Editable_X)
	{
		Editable_X->SetText(FormatFloatAsText(X));
	}

	if (Editable_Y)
	{
		Editable_Y->SetText(FormatFloatAsText(Y));
	}

	if (Editable_Z)
	{
		Editable_Z->SetText(FormatFloatAsText(Z));
	}

	Super::ApplyDisplayValues();
}

void URuntimeGameplaySettingsVectorWidget::CacheDefaultVectorValueIfNeeded()
{
	if (!bHasRuntimeDefaultValue)
	{
		RuntimeDefaultValue = FVector(X, Y, Z);
		bHasRuntimeDefaultValue = true;
	}
}

FVector URuntimeGameplaySettingsVectorWidget::GetDefaultVectorValue() const
{
	return bHasRuntimeDefaultValue ? RuntimeDefaultValue : FVector(X, Y, Z);
}
