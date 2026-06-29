#pragma once

#include "Blueprint/UserWidget.h"
#include "RuntimeGameplaySettingsCategoryPageWidget.generated.h"

class UScrollBox;
class UVerticalBox;
class UWidget;

UCLASS(Blueprintable)
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsCategoryPageWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Category Page")
	UVerticalBox* GetOrCreatePropertiesBox();

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Category Page")
	void ClearProperties();

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Category Page")
	void AddPropertyWidget(UWidget* PropertyWidget);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UScrollBox> Properties_ScrollBox;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> Properties_VBox;
};
