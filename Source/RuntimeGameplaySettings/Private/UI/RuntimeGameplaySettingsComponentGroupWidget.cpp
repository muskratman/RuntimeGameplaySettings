#include "UI/RuntimeGameplaySettingsComponentGroupWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "UI/Elements/RuntimeGameplaySettingsElementWidgetHelpers.h"

void URuntimeGameplaySettingsComponentGroupWidget::SetComponentTitleText(const FText& InTitleText)
{
	EnsureDefaultWidgetTree();
	CurrentTitleText = InTitleText;

	if (ComponentName_TextBlock)
	{
		ComponentName_TextBlock->SetText(CurrentTitleText);
		ApplyTitleVisibility();
	}
}

UVerticalBox* URuntimeGameplaySettingsComponentGroupWidget::GetOrCreatePropertiesBox()
{
	EnsureDefaultWidgetTree();
	return Properties_VBox;
}

void URuntimeGameplaySettingsComponentGroupWidget::ClearProperties()
{
	if (UVerticalBox* PropertiesBox = GetOrCreatePropertiesBox())
	{
		PropertiesBox->ClearChildren();
	}
}

void URuntimeGameplaySettingsComponentGroupWidget::AddPropertyWidget(UWidget* PropertyWidget)
{
	if (!PropertyWidget)
	{
		return;
	}

	if (UVerticalBox* PropertiesBox = GetOrCreatePropertiesBox())
	{
	if (UVerticalBoxSlot* PropertySlot = PropertiesBox->AddChildToVerticalBox(PropertyWidget))
	{
		PropertySlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
		}
	}
}

void URuntimeGameplaySettingsComponentGroupWidget::NativePreConstruct()
{
	EnsureDefaultWidgetTree();
	Super::NativePreConstruct();
}

void URuntimeGameplaySettingsComponentGroupWidget::NativeConstruct()
{
	EnsureDefaultWidgetTree();
	Super::NativeConstruct();

	if (ComponentName_TextBlock)
	{
		ComponentName_TextBlock->SetText(CurrentTitleText);
		ApplyTitleVisibility();
	}
}

void URuntimeGameplaySettingsComponentGroupWidget::EnsureDefaultWidgetTree()
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

void URuntimeGameplaySettingsComponentGroupWidget::BuildDefaultWidgetTree()
{
	using namespace RuntimeGameplaySettingsElementWidgetHelpers;

	if (!WidgetTree)
	{
		return;
	}

	UVerticalBox* RootBox = ConstructWidget<UVerticalBox>(WidgetTree, TEXT("VerticalBox_0"));
	const FText TitleText = CurrentTitleText.IsEmpty() ? INVTEXT("Component") : CurrentTitleText;
	ComponentName_TextBlock = CreateTextBlock(WidgetTree, TEXT("ComponentName_TextBlock"), TitleText, 30.0f);
	Properties_VBox = ConstructWidget<UVerticalBox>(WidgetTree, TEXT("Properties_VBox"));

	if (!RootBox || !ComponentName_TextBlock || !Properties_VBox)
	{
		return;
	}

	if (UVerticalBoxSlot* TitleSlot = RootBox->AddChildToVerticalBox(ComponentName_TextBlock))
	{
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
		TitleSlot->SetHorizontalAlignment(HAlign_Center);
	}
	RootBox->AddChildToVerticalBox(Properties_VBox);

	WidgetTree->RootWidget = RootBox;
}

void URuntimeGameplaySettingsComponentGroupWidget::ApplyTitleVisibility()
{
	if (!ComponentName_TextBlock)
	{
		return;
	}

	const bool bIsDefaultSubCategory =
		CurrentTitleText.ToString().TrimStartAndEnd().Equals(TEXT("Default"), ESearchCase::IgnoreCase);
	ComponentName_TextBlock->SetVisibility(
		bIsDefaultSubCategory ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
}
