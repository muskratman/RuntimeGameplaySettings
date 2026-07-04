#pragma once

#include "Blueprint/UserWidget.h"
#include "RuntimeGameplaySettingsComponentGroupWidget.generated.h"

class UTextBlock;
class UVerticalBox;
class UWidget;

UCLASS(Blueprintable)
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsComponentGroupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Component Group")
	void SetComponentTitleText(const FText& InTitleText);

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Component Group")
	UVerticalBox* GetOrCreatePropertiesBox();

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Component Group")
	void ClearProperties();

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Component Group")
	void AddPropertyWidget(UWidget* PropertyWidget);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ComponentName_TextBlock;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> Properties_VBox;

private:
	void EnsureDefaultWidgetTree();
	void BuildDefaultWidgetTree();
	void ApplyTitleVisibility();

	UPROPERTY(Transient)
	FText CurrentTitleText;
};
