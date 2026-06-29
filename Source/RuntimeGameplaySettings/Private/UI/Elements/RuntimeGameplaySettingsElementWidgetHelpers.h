#pragma once

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScaleBox.h"
#include "Components/ScaleBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/SlateColor.h"
#include "UI/Elements/RuntimeGameplaySettingsResetButtonWidget.h"

class UWidget;

namespace RuntimeGameplaySettingsElementWidgetHelpers
{
	constexpr float LabelWidth = 450.0f;
	constexpr float ValueWidth = 200.0f;
	constexpr float SliderWidth = 300.0f;
	constexpr float LabelFontSize = 27.0f;
	constexpr float SmallButtonWidth = 44.0f;
	constexpr float SmallButtonHeight = 36.0f;

	inline FLinearColor ResetNormalTint()
	{
		return FLinearColor(0.390625f, 0.148280f, 0.020345f, 1.0f);
	}

	inline FLinearColor ResetPressedTint()
	{
		return FLinearColor(0.182292f, 0.069197f, 0.009494f, 1.0f);
	}

	inline FLinearColor AddNormalTint()
	{
		return FLinearColor(0.020182f, 0.322917f, 0.083381f, 1.0f);
	}

	inline FLinearColor AddPressedTint()
	{
		return FLinearColor(0.011719f, 0.187500f, 0.048415f, 1.0f);
	}

	inline FLinearColor DeleteNormalTint()
	{
		return FLinearColor(0.229167f, 0.011936f, 0.013966f, 1.0f);
	}

	inline FLinearColor DeletePressedTint()
	{
		return FLinearColor(0.135417f, 0.007053f, 0.008253f, 1.0f);
	}

	template <typename WidgetType>
	WidgetType* ConstructWidget(UWidgetTree* WidgetTree, const FName& WidgetName)
	{
		return WidgetTree ? WidgetTree->ConstructWidget<WidgetType>(WidgetType::StaticClass(), WidgetName) : nullptr;
	}

	inline void ApplyTextFontSize(UTextBlock* TextBlock, float FontSize)
	{
		if (!TextBlock)
		{
			return;
		}

		FSlateFontInfo FontInfo = TextBlock->GetFont();
		FontInfo.Size = static_cast<int32>(FontSize);
		TextBlock->SetFont(FontInfo);
	}

	inline UTextBlock* CreateTextBlock(UWidgetTree* WidgetTree, const FName& WidgetName, const FText& Text, float FontSize)
	{
		UTextBlock* TextBlock = ConstructWidget<UTextBlock>(WidgetTree, WidgetName);
		if (!TextBlock)
		{
			return nullptr;
		}

		TextBlock->SetText(Text);
		ApplyTextFontSize(TextBlock, FontSize);
		return TextBlock;
	}

	inline USizeBox* CreateSizeBox(UWidgetTree* WidgetTree, const FName& WidgetName, float WidthOverride)
	{
		USizeBox* SizeBox = ConstructWidget<USizeBox>(WidgetTree, WidgetName);
		if (!SizeBox)
		{
			return nullptr;
		}

		if (WidthOverride > 0.0f)
		{
			SizeBox->SetWidthOverride(WidthOverride);
		}
		return SizeBox;
	}

	inline USizeBox* WrapInSizeBox(
		UWidgetTree* WidgetTree,
		const FName& WidgetName,
		UWidget* Content,
		float WidthOverride)
	{
		USizeBox* SizeBox = CreateSizeBox(WidgetTree, WidgetName, WidthOverride);
		if (SizeBox && Content)
		{
			SizeBox->AddChild(Content);
		}
		return SizeBox;
	}

	inline USizeBox* WrapLabelInSizeBox(
		UWidgetTree* WidgetTree,
		UTextBlock* LabelText,
		const FName& SizeBoxName = FName(TEXT("SizeBox_0")),
		const FName& ScaleBoxName = FName(TEXT("ScaleBox_0")),
		float WidthOverride = LabelWidth)
	{
		UScaleBox* ScaleBox = ConstructWidget<UScaleBox>(WidgetTree, ScaleBoxName);
		if (ScaleBox && LabelText)
		{
			ScaleBox->AddChild(LabelText);
			if (UScaleBoxSlot* ScaleBoxSlot = Cast<UScaleBoxSlot>(LabelText->Slot))
			{
				ScaleBoxSlot->SetHorizontalAlignment(HAlign_Left);
			}
		}

		USizeBox* SizeBox = WrapInSizeBox(WidgetTree, SizeBoxName, ScaleBox, WidthOverride);
		if (SizeBox)
		{
			SizeBox->SetMaxDesiredWidth(500.0f);
		}
		return SizeBox;
	}

	inline void AddToHorizontalBox(
		UHorizontalBox* ParentBox,
		UWidget* ChildWidget,
		const FMargin& Padding = FMargin(0.0f),
		EHorizontalAlignment HorizontalAlignment = HAlign_Left,
		EVerticalAlignment VerticalAlignment = VAlign_Center)
	{
		if (!ParentBox || !ChildWidget)
		{
			return;
		}

		if (UHorizontalBoxSlot* Slot = ParentBox->AddChildToHorizontalBox(ChildWidget))
		{
			Slot->SetPadding(Padding);
			Slot->SetHorizontalAlignment(HorizontalAlignment);
			Slot->SetVerticalAlignment(VerticalAlignment);
		}
	}

	inline void AddToVerticalBox(UVerticalBox* ParentBox, UWidget* ChildWidget, const FMargin& Padding = FMargin(0.0f))
	{
		if (!ParentBox || !ChildWidget)
		{
			return;
		}

		if (UVerticalBoxSlot* Slot = ParentBox->AddChildToVerticalBox(ChildWidget))
		{
			Slot->SetPadding(Padding);
		}
	}

	inline void SetButtonContentPadding(UButton* Button, const FMargin& Padding)
	{
		if (!Button)
		{
			return;
		}

		if (UButtonSlot* ButtonSlot = Cast<UButtonSlot>(Button->GetContentSlot()))
		{
			ButtonSlot->SetPadding(Padding);
		}
	}

	inline void ApplyButtonTint(UButton* Button, const FLinearColor& NormalTint, const FLinearColor& PressedTint)
	{
		if (!Button)
		{
			return;
		}

		FButtonStyle ButtonStyle = Button->GetStyle();
		ButtonStyle.Normal.TintColor = FSlateColor(NormalTint);
		ButtonStyle.Hovered.TintColor = FSlateColor(NormalTint);
		ButtonStyle.Pressed.TintColor = FSlateColor(PressedTint);
		ButtonStyle.Disabled.TintColor = FSlateColor(FLinearColor(0.0625f, 0.023725f, 0.003255f, 0.3f));
		Button->SetStyle(ButtonStyle);
	}

	inline void ApplyEditableTextBoxStyle(UEditableTextBox* EditableTextBox, float FontSize = 0.0f)
	{
		if (!EditableTextBox)
		{
			return;
		}

		FEditableTextBoxStyle EditableStyle = EditableTextBox->GetWidgetStyle();
		EditableStyle.TextStyle.SetColorAndOpacity(FSlateColor(FLinearColor::Black));
		if (FontSize > 0.0f)
		{
			FSlateFontInfo FontInfo = EditableStyle.TextStyle.Font;
			FontInfo.Size = static_cast<int32>(FontSize);
			EditableStyle.TextStyle.SetFont(FontInfo);
		}
		EditableTextBox->SetWidgetStyle(EditableStyle);
	}

	inline void ApplySliderThumbSize(USlider* Slider, const FVector2D& ThumbSize)
	{
		if (!Slider)
		{
			return;
		}

		FSliderStyle SliderStyle = Slider->GetWidgetStyle();
		SliderStyle.NormalThumbImage.ImageSize = ThumbSize;
		SliderStyle.HoveredThumbImage.ImageSize = ThumbSize;
		SliderStyle.DisabledThumbImage.ImageSize = ThumbSize;
		Slider->SetWidgetStyle(SliderStyle);
	}

	inline void ApplyCheckBoxImageSize(UCheckBox* CheckBox, const FVector2D& ImageSize)
	{
		if (!CheckBox)
		{
			return;
		}

		FCheckBoxStyle CheckBoxStyle = CheckBox->GetWidgetStyle();
		CheckBoxStyle.UncheckedImage.ImageSize = ImageSize;
		CheckBoxStyle.UncheckedHoveredImage.ImageSize = ImageSize;
		CheckBoxStyle.UncheckedPressedImage.ImageSize = ImageSize;
		CheckBoxStyle.CheckedImage.ImageSize = ImageSize;
		CheckBoxStyle.CheckedHoveredImage.ImageSize = ImageSize;
		CheckBoxStyle.CheckedPressedImage.ImageSize = ImageSize;
		CheckBoxStyle.UndeterminedImage.ImageSize = ImageSize;
		CheckBoxStyle.UndeterminedHoveredImage.ImageSize = ImageSize;
		CheckBoxStyle.UndeterminedPressedImage.ImageSize = ImageSize;
		CheckBox->SetWidgetStyle(CheckBoxStyle);
	}

	inline UButton* CreateTextButton(
		UWidgetTree* WidgetTree,
		const FName& ButtonName,
		const FName& TextName,
		const FText& ButtonText,
		const FLinearColor& NormalTint,
		const FLinearColor& PressedTint,
		float FontSize = 24.0f)
	{
		UButton* Button = ConstructWidget<UButton>(WidgetTree, ButtonName);
		UTextBlock* TextBlock = CreateTextBlock(WidgetTree, TextName, ButtonText, FontSize);
		if (!Button)
		{
			return nullptr;
		}

		if (TextBlock)
		{
			TextBlock->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			Button->AddChild(TextBlock);
		}
		ApplyButtonTint(Button, NormalTint, PressedTint);
		return Button;
	}

	inline USizeBox* CreateSmallButtonBox(UWidgetTree* WidgetTree, const FName& WidgetName, UWidget* Content)
	{
		USizeBox* SizeBox = CreateSizeBox(WidgetTree, WidgetName, SmallButtonWidth);
		if (!SizeBox)
		{
			return nullptr;
		}

		SizeBox->SetHeightOverride(SmallButtonHeight);
		if (Content)
		{
			SizeBox->AddChild(Content);
		}
		return SizeBox;
	}

	inline URuntimeGameplaySettingsResetButtonWidget* CreateResetButtonWidget(UWidgetTree* WidgetTree)
	{
		return WidgetTree
			? WidgetTree->ConstructWidget<URuntimeGameplaySettingsResetButtonWidget>(
				URuntimeGameplaySettingsResetButtonWidget::StaticClass(),
				TEXT("ResetButtonWidget"))
			: nullptr;
	}
}
