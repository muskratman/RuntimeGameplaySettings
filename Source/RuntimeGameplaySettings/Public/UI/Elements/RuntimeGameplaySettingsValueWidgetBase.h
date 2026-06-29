#pragma once

#include "Blueprint/UserWidget.h"
#include "RuntimeGameplaySettingsValueWidgetBase.generated.h"

class UButton;
class UTextBlock;
class URuntimeGameplaySettingsResetButtonWidget;

UCLASS(Abstract)
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsValueWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Value")
	void SetParameterName(const FText& InParameterName);

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Value")
	void ResetToDefaultValue();

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_Name;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> ResetButt;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<URuntimeGameplaySettingsResetButtonWidget> ResetButtonWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Value")
	FText ParameterName = INVTEXT("Parameter Name");

	virtual void ApplyDisplayValues();
	virtual void ResetValueToDefault();
	virtual bool IsValueAtDefault() const;
	virtual void BuildDefaultWidgetTree();
	void EnsureDefaultWidgetTree();
	void UpdateResetButtonState();

	static FText FormatFloatAsText(float InValue);
	static FString FormatFloatAsString(float InValue);
	static bool TryParseFloatText(const FText& InText, float& OutValue);

	bool bIsSynchronizing = false;

private:
	UFUNCTION()
	void HandleResetReleased();

	UFUNCTION()
	void HandleResetWidgetReleased();
};
