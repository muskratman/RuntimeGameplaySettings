#include "UI/RuntimeGameplaySettingsCategorySwitcherWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "UI/Elements/RuntimeGameplaySettingsElementWidgetHelpers.h"

void URuntimeGameplaySettingsCategorySwitcherWidget::SetTitleText(const FText& InTitleText)
{
	EnsureDefaultWidgetTree();
	CurrentTitleText = InTitleText;

	if (Title_TextBlock)
	{
		Title_TextBlock->SetText(CurrentTitleText);
	}
}

UVerticalBox* URuntimeGameplaySettingsCategorySwitcherWidget::GetOrCreatePropertiesBox()
{
	EnsureDefaultWidgetTree();
	return SwitcherTab_VBox;
}

void URuntimeGameplaySettingsCategorySwitcherWidget::ClearProperties()
{
	if (UVerticalBox* PropertiesBox = GetOrCreatePropertiesBox())
	{
		PropertiesBox->ClearChildren();
	}
}

void URuntimeGameplaySettingsCategorySwitcherWidget::AddPropertyWidget(UWidget* PropertyWidget)
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

void URuntimeGameplaySettingsCategorySwitcherWidget::NativeConstruct()
{
	EnsureDefaultWidgetTree();
	Super::NativeConstruct();

	if (Title_TextBlock)
	{
		Title_TextBlock->SetText(CurrentTitleText);
	}
}

void URuntimeGameplaySettingsCategorySwitcherWidget::NativePreConstruct()
{
	EnsureDefaultWidgetTree();
	Super::NativePreConstruct();
}

void URuntimeGameplaySettingsCategorySwitcherWidget::EnsureDefaultWidgetTree()
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

void URuntimeGameplaySettingsCategorySwitcherWidget::BuildDefaultWidgetTree()
{
	using namespace RuntimeGameplaySettingsElementWidgetHelpers;

	if (!WidgetTree)
	{
		return;
	}

	UScrollBox* TabScrollBox = ConstructWidget<UScrollBox>(WidgetTree, TEXT("TabScrollBox"));
	UVerticalBox* RootBox = ConstructWidget<UVerticalBox>(WidgetTree, TEXT("VerticalBox_2"));
	const FText TitleText = CurrentTitleText.IsEmpty() ? INVTEXT("Common Settings") : CurrentTitleText;
	Title_TextBlock = CreateTextBlock(WidgetTree, TEXT("Title_TextBlock"), TitleText, 30.0f);
	SwitcherTab_VBox = ConstructWidget<UVerticalBox>(WidgetTree, TEXT("SwitcherTab_VBox"));

	if (!TabScrollBox || !RootBox || !SwitcherTab_VBox)
	{
		return;
	}

	TabScrollBox->SetScrollbarPadding(FMargin(20.0f));
	if (UVerticalBoxSlot* TitleSlot = RootBox->AddChildToVerticalBox(Title_TextBlock))
	{
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 20.0f));
		TitleSlot->SetHorizontalAlignment(HAlign_Center);
	}
	RootBox->AddChildToVerticalBox(SwitcherTab_VBox);
	TabScrollBox->AddChild(RootBox);

	WidgetTree->RootWidget = TabScrollBox;
}
