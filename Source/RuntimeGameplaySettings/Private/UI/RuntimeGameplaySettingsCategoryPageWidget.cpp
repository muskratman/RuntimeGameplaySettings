#include "UI/RuntimeGameplaySettingsCategoryPageWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"

UVerticalBox* URuntimeGameplaySettingsCategoryPageWidget::GetOrCreatePropertiesBox()
{
	if (Properties_VBox)
	{
		return Properties_VBox;
	}

	if (!WidgetTree)
	{
		return nullptr;
	}

	if (!Properties_ScrollBox)
	{
		Properties_ScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("Properties_ScrollBox"));
		WidgetTree->RootWidget = Properties_ScrollBox;
	}

	Properties_VBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Properties_VBox"));
	Properties_ScrollBox->AddChild(Properties_VBox);
	return Properties_VBox;
}

void URuntimeGameplaySettingsCategoryPageWidget::ClearProperties()
{
	if (UVerticalBox* PropertiesBox = GetOrCreatePropertiesBox())
	{
		PropertiesBox->ClearChildren();
	}
}

void URuntimeGameplaySettingsCategoryPageWidget::AddPropertyWidget(UWidget* PropertyWidget)
{
	if (UVerticalBox* PropertiesBox = GetOrCreatePropertiesBox())
	{
		PropertiesBox->AddChildToVerticalBox(PropertyWidget);
	}
}

void URuntimeGameplaySettingsCategoryPageWidget::NativeConstruct()
{
	Super::NativeConstruct();
	GetOrCreatePropertiesBox();
}
