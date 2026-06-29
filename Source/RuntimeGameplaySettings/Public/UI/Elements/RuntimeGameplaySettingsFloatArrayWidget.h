#pragma once

#include "UI/Elements/RuntimeGameplaySettingsArrayWidgetBase.h"
#include "RuntimeGameplaySettingsFloatArrayWidget.generated.h"

UCLASS(Blueprintable)
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsFloatArrayWidget : public URuntimeGameplaySettingsArrayWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Float Array")
	void SetFloatValues(const TArray<float>& InValues);

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Float Array")
	void SetDefaultFloatValues(const TArray<float>& InValues);

	UFUNCTION(BlueprintPure, Category="RuntimeGameplaySettings Float Array")
	TArray<float> GetFloatValues() const;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Float Array")
	TArray<float> DefaultValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Float Array")
	float NewElementDefaultValue = 0.0f;

	virtual int32 GetArrayValueCount() const override;
	virtual void AppendDefaultArrayValue() override;
	virtual void RemoveArrayValueAt(int32 ElementIndex) override;
	virtual void ResetArrayValuesToDefault() override;
	virtual bool IsArrayValueAtDefault() const override;
	virtual void SynchronizeArrayValuesFromElementWidgets() override;
	virtual void InitializeElementWidget(URuntimeGameplaySettingsArrayElementWidgetBase* ElementWidget, int32 ElementIndex) override;
	virtual TSubclassOf<URuntimeGameplaySettingsArrayElementWidgetBase> GetResolvedElementWidgetClass() const override;

private:
	TArray<float> CurrentValues;
	TArray<float> RuntimeDefaultValues;
	bool bHasInitializedValues = false;
	bool bHasRuntimeDefaultValues = false;
};
