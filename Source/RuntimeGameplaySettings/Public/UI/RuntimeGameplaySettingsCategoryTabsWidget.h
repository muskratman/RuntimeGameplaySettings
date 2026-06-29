#pragma once

#include "Blueprint/UserWidget.h"
#include "RuntimeGameplaySettingsCategoryTabsWidget.generated.h"

class URuntimeGameplaySettingsButtonWidget;
class USizeBox;
class UVerticalBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FRuntimeGameplaySettingsCategorySelectedEvent,
	int32,
	CategoryIndex);

UCLASS(Blueprintable)
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsCategoryTabsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="RuntimeGameplaySettings Categories")
	FRuntimeGameplaySettingsCategorySelectedEvent OnCategorySelected;

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Categories")
	void ClearCategories();

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Categories")
	URuntimeGameplaySettingsButtonWidget* AddCategory(
		const FText& CategoryText,
		const FName& WidgetName,
		TSubclassOf<URuntimeGameplaySettingsButtonWidget> ButtonWidgetClass);

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Categories")
	void ActivateCategory(int32 CategoryIndex);

	UFUNCTION(BlueprintPure, Category="RuntimeGameplaySettings Categories")
	UVerticalBox* GetCategoriesBox() const;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<USizeBox> Category_SizeBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> Categories_VBox;

private:
	UFUNCTION()
	void HandleCategoryButtonReleased(URuntimeGameplaySettingsButtonWidget* ButtonWidget);

	UPROPERTY(Transient)
	TArray<TObjectPtr<URuntimeGameplaySettingsButtonWidget>> CategoryButtons;

	int32 ActiveCategoryIndex = INDEX_NONE;

	void EnsureDefaultWidgetTree();
	void BuildDefaultWidgetTree();
};
