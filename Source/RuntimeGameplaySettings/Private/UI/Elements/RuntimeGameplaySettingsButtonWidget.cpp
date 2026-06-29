#include "UI/Elements/RuntimeGameplaySettingsButtonWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/ScaleBox.h"
#include "Components/TextBlock.h"
#include "Math/UnrealMathUtility.h"
#include "UI/Elements/RuntimeGameplaySettingsElementWidgetHelpers.h"

void URuntimeGameplaySettingsButtonWidget::SetButtonText(const FText& InButtonText)
{
	EnsureDefaultWidgetTree();
	Text = InButtonText;
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsButtonWidget::SetButtonTint(const FLinearColor& InButtonTint)
{
	EnsureDefaultWidgetTree();
	ButtonTint = InButtonTint;
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsButtonWidget::NativePreConstruct()
{
	EnsureDefaultWidgetTree();
	Super::NativePreConstruct();
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsButtonWidget::NativeOnInitialized()
{
	EnsureDefaultWidgetTree();
	Super::NativeOnInitialized();

	if (Button)
	{
		Button->OnReleased.AddDynamic(this, &URuntimeGameplaySettingsButtonWidget::HandleButtonReleased);
	}

	ApplyDisplayValues();
}

void URuntimeGameplaySettingsButtonWidget::EnsureDefaultWidgetTree()
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

void URuntimeGameplaySettingsButtonWidget::BuildDefaultWidgetTree()
{
	if (!WidgetTree)
	{
		return;
	}

	Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("Button"));
	UScaleBox* ScaleBox = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass(), TEXT("ScaleBox_121"));
	ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ButtonText"));

	if (ScaleBox && ButtonText)
	{
		ScaleBox->AddChild(ButtonText);
	}

	if (Button)
	{
		Button->AddChild(ScaleBox ? static_cast<UWidget*>(ScaleBox) : static_cast<UWidget*>(ButtonText));
		WidgetTree->RootWidget = Button;
	}
}

void URuntimeGameplaySettingsButtonWidget::HandleButtonReleased()
{
	OnRuntimeGameplaySettingsButtonReleased.Broadcast(this);
}

void URuntimeGameplaySettingsButtonWidget::ApplyDisplayValues()
{
	if (ButtonText)
	{
		ButtonText->SetText(Text);
		RuntimeGameplaySettingsElementWidgetHelpers::ApplyTextFontSize(ButtonText, 40.0f);
	}

	if (Button)
	{
		FButtonStyle ButtonStyle = Button->GetStyle();
		ButtonStyle.Normal.TintColor = FSlateColor(ButtonTint);
		ButtonStyle.Hovered.TintColor = FSlateColor(ButtonTint);
		ButtonStyle.Pressed.TintColor = FSlateColor(BuildPressedTint());
		Button->SetStyle(ButtonStyle);
	}
}

FLinearColor URuntimeGameplaySettingsButtonWidget::BuildPressedTint() const
{
	constexpr float PressedTintMultiplier = 0.75f;

	FLinearColor PressedTint = ButtonTint;
	PressedTint.R = FMath::Clamp(PressedTint.R * PressedTintMultiplier, 0.0f, 1.0f);
	PressedTint.G = FMath::Clamp(PressedTint.G * PressedTintMultiplier, 0.0f, 1.0f);
	PressedTint.B = FMath::Clamp(PressedTint.B * PressedTintMultiplier, 0.0f, 1.0f);
	return PressedTint;
}
