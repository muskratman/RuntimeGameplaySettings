#pragma once

#include "Components/MultiLineEditableTextBox.h"
#include "Curves/CurveFloat.h"
#include "UI/Elements/RuntimeGameplaySettingsValueWidgetBase.h"
#include "RuntimeGameplaySettingsCurveWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS(Blueprintable)
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsCurveWidget : public URuntimeGameplaySettingsValueWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Curve")
	void SetCurveValue(const FRuntimeFloatCurve& InCurveValue);

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Curve")
	void SetDefaultCurveValue(const FRuntimeFloatCurve& InCurveValue);

	UFUNCTION(BlueprintPure, Category="RuntimeGameplaySettings Curve")
	FRuntimeFloatCurve GetCurveValue() const;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void ApplyDisplayValues() override;
	virtual void ResetValueToDefault() override;
	virtual bool IsValueAtDefault() const override;
	virtual void BuildDefaultWidgetTree() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMultiLineEditableTextBox> Editable_Curve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Curve")
	FRuntimeFloatCurve DefaultCurve;

private:
	UFUNCTION()
	void HandleCurveCommitted(const FText& InText, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void HandleCurveTextChanged(const FText& InText);

	const FRuntimeFloatCurve& GetDefaultCurveValue() const;
	bool AreCurvesEquivalent(const FRuntimeFloatCurve& FirstCurve, const FRuntimeFloatCurve& SecondCurve) const;
	FText FormatCurveAsText(const FRuntimeFloatCurve& InCurveValue) const;
	bool TryParseCurve(const FText& InText, FRuntimeFloatCurve& OutCurveValue) const;
	bool TryParseCurvePoint(const FString& InText, float& OutTime, float& OutValue) const;

	FRuntimeFloatCurve CurrentCurve;
	FRuntimeFloatCurve RuntimeDefaultCurve;
	bool bHasInitializedCurve = false;
	bool bHasRuntimeDefaultCurve = false;
};
