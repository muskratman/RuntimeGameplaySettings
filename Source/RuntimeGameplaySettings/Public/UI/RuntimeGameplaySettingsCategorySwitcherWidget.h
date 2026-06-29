#pragma once

#include "Blueprint/UserWidget.h"
#include "RuntimeGameplaySettingsCategorySwitcherWidget.generated.h"

class UTextBlock;
class UVerticalBox;
class UWidget;

UCLASS(Blueprintable)
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsCategorySwitcherWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Category Switcher")
	void SetTitleText(const FText& InTitleText);

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Category Switcher")
	UVerticalBox* GetOrCreatePropertiesBox();

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Category Switcher")
	void ClearProperties();

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Category Switcher")
	void AddPropertyWidget(UWidget* PropertyWidget);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Title_TextBlock;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> SwitcherTab_VBox;

	UPROPERTY(Transient)
	FText CurrentTitleText;

	void EnsureDefaultWidgetTree();
	void BuildDefaultWidgetTree();
};
