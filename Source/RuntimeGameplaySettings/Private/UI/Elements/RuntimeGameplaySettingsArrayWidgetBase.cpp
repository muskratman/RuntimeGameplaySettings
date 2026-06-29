#include "UI/Elements/RuntimeGameplaySettingsArrayWidgetBase.h"

#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Templates/UnrealTemplate.h"
#include "UI/Elements/RuntimeGameplaySettingsArrayElementWidgetBase.h"
#include "UI/Elements/RuntimeGameplaySettingsElementWidgetHelpers.h"

void URuntimeGameplaySettingsArrayWidgetBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	ApplyDisplayValues();
	RebuildArrayElements();
}

void URuntimeGameplaySettingsArrayWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (AddButt)
	{
		AddButt->OnReleased.AddDynamic(this, &URuntimeGameplaySettingsArrayWidgetBase::HandleAddReleased);
	}

	ApplyDisplayValues();
	RebuildArrayElements();
}

void URuntimeGameplaySettingsArrayWidgetBase::ApplyDisplayValues()
{
	Super::ApplyDisplayValues();
}

void URuntimeGameplaySettingsArrayWidgetBase::ResetValueToDefault()
{
	if (bIsRebuildingArray)
	{
		return;
	}

	SynchronizeArrayValuesFromElementWidgets();
	ResetArrayValuesToDefault();
	RebuildArrayElements();
}

bool URuntimeGameplaySettingsArrayWidgetBase::IsValueAtDefault() const
{
	return IsArrayValueAtDefault();
}

void URuntimeGameplaySettingsArrayWidgetBase::BuildDefaultWidgetTree()
{
	using namespace RuntimeGameplaySettingsElementWidgetHelpers;

	if (!WidgetTree)
	{
		return;
	}

	UBorder* RootBorder = ConstructWidget<UBorder>(WidgetTree, TEXT("Border_0"));
	UHorizontalBox* HeaderBox = ConstructWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_2"));
	UHorizontalBox* ArrayBox = ConstructWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_63"));
	Txt_Name = CreateTextBlock(WidgetTree, TEXT("Txt_Name"), INVTEXT("Param Name"), LabelFontSize);
	Array_VerticalBox = ConstructWidget<UVerticalBox>(WidgetTree, TEXT("Array_VerticalBox"));
	AddButt = CreateTextButton(
		WidgetTree,
		TEXT("AddButt"),
		TEXT("TextBlock_51"),
		INVTEXT("+"),
		AddNormalTint(),
		AddPressedTint(),
		35.0f);
	ResetButtonWidget = CreateResetButtonWidget(WidgetTree);
	SetButtonContentPadding(AddButt, FMargin(0.0f, -5.0f, 0.0f, -5.0f));

	if (!RootBorder || !HeaderBox || !ArrayBox)
	{
		return;
	}

	AddToHorizontalBox(ArrayBox, Array_VerticalBox, FMargin(0.0f), HAlign_Left);

	AddToHorizontalBox(
		HeaderBox,
		WrapLabelInSizeBox(WidgetTree, Txt_Name),
		FMargin(0.0f),
		HAlign_Left,
		VAlign_Top);
	if (UHorizontalBoxSlot* ArraySlot = HeaderBox->AddChildToHorizontalBox(ArrayBox))
	{
		ArraySlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		ArraySlot->SetHorizontalAlignment(HAlign_Right);
	}
	AddToHorizontalBox(HeaderBox, AddButt, FMargin(10.0f, 0.0f, 0.0f, 0.0f), HAlign_Right, VAlign_Top);
	AddToHorizontalBox(HeaderBox, ResetButtonWidget, FMargin(10.0f, 0.0f, 0.0f, 0.0f), HAlign_Right);

	RootBorder->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 4.0f));
	RootBorder->AddChild(HeaderBox);
	WidgetTree->RootWidget = RootBorder;
}

void URuntimeGameplaySettingsArrayWidgetBase::RebuildArrayElements()
{
	if (!Array_VerticalBox)
	{
		return;
	}

	TGuardValue<bool> RebuildGuard(bIsRebuildingArray, true);

	Array_VerticalBox->ClearChildren();
	ElementWidgets.Reset();

	const TSubclassOf<URuntimeGameplaySettingsArrayElementWidgetBase> ResolvedElementWidgetClass = GetResolvedElementWidgetClass();
	if (!ResolvedElementWidgetClass)
	{
		return;
	}

	const int32 ArrayValueCount = GetArrayValueCount();
	for (int32 ElementIndex = 0; ElementIndex < ArrayValueCount; ++ElementIndex)
	{
		URuntimeGameplaySettingsArrayElementWidgetBase* ElementWidget =
			CreateWidget<URuntimeGameplaySettingsArrayElementWidgetBase>(this, ResolvedElementWidgetClass);
		if (!ElementWidget)
		{
			continue;
		}

		ElementWidget->SetElementIndex(ElementIndex);
		ElementWidget->OnRuntimeGameplaySettingsArrayElementDeleteRequested.AddUniqueDynamic(
			this,
			&URuntimeGameplaySettingsArrayWidgetBase::HandleElementDeleteRequested);
		ElementWidget->OnRuntimeGameplaySettingsArrayElementValueChanged.AddUniqueDynamic(
			this,
			&URuntimeGameplaySettingsArrayWidgetBase::HandleElementValueChanged);
		InitializeElementWidget(ElementWidget, ElementIndex);

		ElementWidgets.Add(ElementWidget);
		Array_VerticalBox->AddChildToVerticalBox(ElementWidget);
	}

	UpdateResetButtonState();
}

int32 URuntimeGameplaySettingsArrayWidgetBase::GetArrayValueCount() const
{
	return 0;
}

void URuntimeGameplaySettingsArrayWidgetBase::AppendDefaultArrayValue()
{
}

void URuntimeGameplaySettingsArrayWidgetBase::RemoveArrayValueAt(int32 ElementIndex)
{
	(void)ElementIndex;
}

void URuntimeGameplaySettingsArrayWidgetBase::ResetArrayValuesToDefault()
{
}

bool URuntimeGameplaySettingsArrayWidgetBase::IsArrayValueAtDefault() const
{
	return true;
}

void URuntimeGameplaySettingsArrayWidgetBase::SynchronizeArrayValuesFromElementWidgets()
{
}

void URuntimeGameplaySettingsArrayWidgetBase::InitializeElementWidget(
	URuntimeGameplaySettingsArrayElementWidgetBase* ElementWidget,
	int32 ElementIndex)
{
	(void)ElementWidget;
	(void)ElementIndex;
}

TSubclassOf<URuntimeGameplaySettingsArrayElementWidgetBase> URuntimeGameplaySettingsArrayWidgetBase::GetResolvedElementWidgetClass() const
{
	return ElementWidgetClass;
}

void URuntimeGameplaySettingsArrayWidgetBase::HandleAddReleased()
{
	if (bIsRebuildingArray)
	{
		return;
	}

	SynchronizeArrayValuesFromElementWidgets();
	AppendDefaultArrayValue();
	RebuildArrayElements();
	UpdateResetButtonState();
}

void URuntimeGameplaySettingsArrayWidgetBase::HandleElementDeleteRequested(URuntimeGameplaySettingsArrayElementWidgetBase* ElementWidget)
{
	if (bIsRebuildingArray || !ElementWidget)
	{
		return;
	}

	const int32 ElementIndex = ElementWidget->GetElementIndex();
	SynchronizeArrayValuesFromElementWidgets();
	RemoveArrayValueAt(ElementIndex);
	RebuildArrayElements();
	UpdateResetButtonState();
}

void URuntimeGameplaySettingsArrayWidgetBase::HandleElementValueChanged(URuntimeGameplaySettingsArrayElementWidgetBase* ElementWidget)
{
	(void)ElementWidget;
	UpdateResetButtonState();
}
