#pragma once

#include "Components/EditableTextBox.h"
#include "UI/Elements/RuntimeGameplaySettingsValueWidgetBase.h"
#include "RuntimeGameplaySettingsParameterWidget.generated.h"

class USlider;
class UTextBlock;
class UButton;

UCLASS(Blueprintable)
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsParameterWidget : public URuntimeGameplaySettingsValueWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Parameter")
	void SetParameterValue(float InValue);

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Parameter")
	void SetDefaultParameterValue(float InValue);

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Parameter")
	void SetParameterRange(float InMinValue, float InMaxValue, float InStepSize = 0.01f);

	UFUNCTION(BlueprintPure, Category="RuntimeGameplaySettings Parameter")
	float GetParameterValue() const;

	UFUNCTION(BlueprintPure, Category="RuntimeGameplaySettings Parameter")
	float GetEditableParameterValue() const;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void ApplyDisplayValues() override;
	virtual void ResetValueToDefault() override;
	virtual bool IsValueAtDefault() const override;
	virtual void BuildDefaultWidgetTree() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<USlider> Slider_Param;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> Editable_Param;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Parameter")
	float DefaultValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Parameter")
	float MinValue = -10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Parameter")
	float MaxValue = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Parameter", meta=(ClampMin=0.0))
	float StepSize = 0.01f;

private:
	UFUNCTION()
	void HandleSliderValueChanged(float InValue);

	UFUNCTION()
	void HandleEditableTextCommitted(const FText& InText, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void HandleEditableTextChanged(const FText& InText);

	void ApplyParameterMetadata();
	void SyncWidgetsToCurrentValue();
	float SanitizeValue(float InValue) const;
	float GetDefaultParameterValue() const;

	float CurrentValue = 0.0f;
	float RuntimeDefaultValue = 0.0f;
	bool bHasInitializedValue = false;
	bool bHasRuntimeDefaultValue = false;
};
