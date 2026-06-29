#pragma once

#include "Components/EditableTextBox.h"
#include "UI/Elements/RuntimeGameplaySettingsArrayElementWidgetBase.h"
#include "RuntimeGameplaySettingsFloatArrayElementWidget.generated.h"

UCLASS(Blueprintable)
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsFloatArrayElementWidget : public URuntimeGameplaySettingsArrayElementWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Float Array")
	void SetFloatValue(float InValue);

	UFUNCTION(BlueprintPure, Category="RuntimeGameplaySettings Float Array")
	float GetFloatValue() const;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void BuildDefaultWidgetTree() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> Editable_Value;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Float Array")
	float DefaultValue = 0.0f;

private:
	UFUNCTION()
	void HandleValueCommitted(const FText& InText, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void HandleValueChanged(const FText& InText);

	void ApplyDisplayValues();
	bool TryParseValue(const FText& InText, float& OutValue) const;
	FString FormatFloat(float InValue) const;

	float CurrentValue = 0.0f;
	bool bHasInitializedValue = false;
	bool bIsSynchronizing = false;
};
