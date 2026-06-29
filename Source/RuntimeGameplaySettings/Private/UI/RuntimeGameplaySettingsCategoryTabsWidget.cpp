#include "UI/RuntimeGameplaySettingsCategoryTabsWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/SlateTypes.h"
#include "UI/Elements/RuntimeGameplaySettingsElementWidgetHelpers.h"
#include "UI/Elements/RuntimeGameplaySettingsButtonWidget.h"

void URuntimeGameplaySettingsCategoryTabsWidget::ClearCategories()
{
	EnsureDefaultWidgetTree();

	if (Categories_VBox)
	{
		Categories_VBox->ClearChildren();
	}

	CategoryButtons.Reset();
	ActiveCategoryIndex = INDEX_NONE;
}

URuntimeGameplaySettingsButtonWidget* URuntimeGameplaySettingsCategoryTabsWidget::AddCategory(
	const FText& CategoryText,
	const FName& WidgetName,
	TSubclassOf<URuntimeGameplaySettingsButtonWidget> ButtonWidgetClass)
{
	EnsureDefaultWidgetTree();

	if (!Categories_VBox || !ButtonWidgetClass || !WidgetTree)
	{
		return nullptr;
	}

	URuntimeGameplaySettingsButtonWidget* ButtonWidget =
		WidgetTree->ConstructWidget<URuntimeGameplaySettingsButtonWidget>(ButtonWidgetClass, WidgetName);
	if (!ButtonWidget)
	{
		return nullptr;
	}

	ButtonWidget->Initialize();
	ButtonWidget->SetVisibility(ESlateVisibility::Visible);
	ButtonWidget->SetButtonText(CategoryText);
	ButtonWidget->OnRuntimeGameplaySettingsButtonReleased.AddUniqueDynamic(
		this,
		&URuntimeGameplaySettingsCategoryTabsWidget::HandleCategoryButtonReleased);

	if (UVerticalBoxSlot* ButtonSlot = Categories_VBox->AddChildToVerticalBox(ButtonWidget))
	{
		ButtonSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
	}
	Categories_VBox->InvalidateLayoutAndVolatility();
	InvalidateLayoutAndVolatility();
	ForceLayoutPrepass();

	CategoryButtons.Add(ButtonWidget);
	return ButtonWidget;
}

void URuntimeGameplaySettingsCategoryTabsWidget::ActivateCategory(int32 CategoryIndex)
{
	EnsureDefaultWidgetTree();
	ActiveCategoryIndex = CategoryButtons.IsValidIndex(CategoryIndex) ? CategoryIndex : INDEX_NONE;
}

UVerticalBox* URuntimeGameplaySettingsCategoryTabsWidget::GetCategoriesBox() const
{
	return Categories_VBox;
}

void URuntimeGameplaySettingsCategoryTabsWidget::NativePreConstruct()
{
	EnsureDefaultWidgetTree();
	Super::NativePreConstruct();
}

void URuntimeGameplaySettingsCategoryTabsWidget::NativeOnInitialized()
{
	EnsureDefaultWidgetTree();
	Super::NativeOnInitialized();
}

void URuntimeGameplaySettingsCategoryTabsWidget::HandleCategoryButtonReleased(
	URuntimeGameplaySettingsButtonWidget* ButtonWidget)
{
	const int32 CategoryIndex = CategoryButtons.IndexOfByKey(ButtonWidget);
	if (CategoryIndex != INDEX_NONE)
	{
		OnCategorySelected.Broadcast(CategoryIndex);
	}
}

void URuntimeGameplaySettingsCategoryTabsWidget::EnsureDefaultWidgetTree()
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

void URuntimeGameplaySettingsCategoryTabsWidget::BuildDefaultWidgetTree()
{
	using namespace RuntimeGameplaySettingsElementWidgetHelpers;

	if (!WidgetTree)
	{
		return;
	}

	Category_SizeBox = ConstructWidget<USizeBox>(WidgetTree, TEXT("Category_SizeBox"));
	UScrollBox* ScrollBox = ConstructWidget<UScrollBox>(WidgetTree, TEXT("ScrollBox_55"));
	Categories_VBox = ConstructWidget<UVerticalBox>(WidgetTree, TEXT("Categories_VBox"));
	if (!Category_SizeBox || !ScrollBox || !Categories_VBox)
	{
		return;
	}

	Category_SizeBox->SetWidthOverride(270.0f);
	ScrollBox->SetConsumeMouseWheel(EConsumeMouseWheel::Always);
	ScrollBox->SetScrollbarThickness(FVector2D(12.0f, 9.0f));
	ScrollBox->SetScrollbarPadding(FMargin(5.0f, 5.0f, 2.0f, 5.0f));
	ScrollBox->SetRenderScale(FVector2D(-1.0f, 1.0f));
	Categories_VBox->SetRenderScale(FVector2D(-1.0f, 1.0f));
	Categories_VBox->SetVisibility(ESlateVisibility::Visible);

	ScrollBox->AddChild(Categories_VBox);
	Category_SizeBox->AddChild(ScrollBox);
	SetVisibility(ESlateVisibility::Visible);
	WidgetTree->RootWidget = Category_SizeBox;
}
