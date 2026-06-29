#include "UI/Elements/RuntimeGameplaySettingsResetButtonWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "UI/Elements/RuntimeGameplaySettingsElementWidgetHelpers.h"

void URuntimeGameplaySettingsResetButtonWidget::SetResetEnabled(bool bInIsEnabled)
{
	EnsureDefaultWidgetTree();
	if (ResetButt)
	{
		ResetButt->SetIsEnabled(bInIsEnabled);
	}
}

void URuntimeGameplaySettingsResetButtonWidget::NativePreConstruct()
{
	EnsureDefaultWidgetTree();
	Super::NativePreConstruct();
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsResetButtonWidget::NativeOnInitialized()
{
	EnsureDefaultWidgetTree();
	Super::NativeOnInitialized();

	if (ResetButt)
	{
		ResetButt->OnReleased.AddDynamic(this, &URuntimeGameplaySettingsResetButtonWidget::HandleResetReleased);
	}

	ApplyDisplayValues();
}

void URuntimeGameplaySettingsResetButtonWidget::HandleResetReleased()
{
	OnRuntimeGameplaySettingsResetButtonReleased.Broadcast();
}

void URuntimeGameplaySettingsResetButtonWidget::EnsureDefaultWidgetTree()
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

void URuntimeGameplaySettingsResetButtonWidget::BuildDefaultWidgetTree()
{
	if (!WidgetTree)
	{
		return;
	}

	ResetButt = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ResetButt"));
	UImage* ResetIcon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("ResetIcon"));
	UTexture2D* ResetTexture = LoadObject<UTexture2D>(
		nullptr,
		TEXT("/RuntimeGameplaySettings/Textures/T_ResetIcon.T_ResetIcon"));

	if (ResetButt && ResetIcon && ResetTexture)
	{
		ResetIcon->SetBrushFromTexture(ResetTexture);
		ResetIcon->SetDesiredSizeOverride(FVector2D(40.0f, 40.0f));
		ResetButt->AddChild(ResetIcon);
		RuntimeGameplaySettingsElementWidgetHelpers::SetButtonContentPadding(ResetButt, FMargin(0.0f));
		if (UButtonSlot* ResetIconSlot = Cast<UButtonSlot>(ResetIcon->Slot))
		{
			ResetIconSlot->SetHorizontalAlignment(HAlign_Center);
			ResetIconSlot->SetVerticalAlignment(VAlign_Center);
		}
		WidgetTree->RootWidget = ResetButt;
	}
	else if (ResetButt)
	{
		ResetText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ResetText"));
		if (ResetText)
		{
			ResetButt->AddChild(ResetText);
		}
		RuntimeGameplaySettingsElementWidgetHelpers::SetButtonContentPadding(ResetButt, FMargin(0.0f));
		if (ResetText)
		{
			if (UButtonSlot* ResetTextSlot = Cast<UButtonSlot>(ResetText->Slot))
			{
				ResetTextSlot->SetHorizontalAlignment(HAlign_Center);
				ResetTextSlot->SetVerticalAlignment(VAlign_Center);
			}
		}
		WidgetTree->RootWidget = ResetButt;
	}
}

void URuntimeGameplaySettingsResetButtonWidget::ApplyDisplayValues()
{
	using namespace RuntimeGameplaySettingsElementWidgetHelpers;

	if (ResetText)
	{
		ResetText->SetText(INVTEXT("R"));
		ApplyTextFontSize(ResetText, 24.0f);
	}

	ApplyButtonTint(ResetButt, ResetNormalTint(), ResetPressedTint());
}
