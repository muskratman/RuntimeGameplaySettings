#pragma once

#include "UI/Elements/RuntimeGameplaySettingsValueWidgetBase.h"
#include "RuntimeGameplaySettingsCheckboxWidget.generated.h"

class UCheckBox;
class UTextBlock;
class UButton;

UCLASS(Blueprintable)
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsCheckboxWidget : public URuntimeGameplaySettingsValueWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Checkbox")
	void SetCheckboxValue(bool bInValue);

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Checkbox")
	void SetDefaultCheckboxValue(bool bInValue);

	UFUNCTION(BlueprintPure, Category="RuntimeGameplaySettings Checkbox")
	bool GetCheckboxValue() const;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void ApplyDisplayValues() override;
	virtual void ResetValueToDefault() override;
	virtual bool IsValueAtDefault() const override;
	virtual void BuildDefaultWidgetTree() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCheckBox> Checkbox_Value;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Checkbox")
	bool bDefaultValue = false;

private:
	UFUNCTION()
	void HandleCheckboxValueChanged(bool bIsChecked);

	bool GetDefaultCheckboxValue() const;

	bool bCurrentValue = false;
	bool bRuntimeDefaultValue = false;
	bool bHasInitializedValue = false;
	bool bHasRuntimeDefaultValue = false;
};
