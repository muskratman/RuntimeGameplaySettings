#pragma once

#include "UI/Elements/RuntimeGameplaySettingsValueWidgetBase.h"
#include "RuntimeGameplaySettingsArrayWidgetBase.generated.h"

class UButton;
class URuntimeGameplaySettingsArrayElementWidgetBase;
class UTextBlock;
class UVerticalBox;

UCLASS(Abstract)
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsArrayWidgetBase : public URuntimeGameplaySettingsValueWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void ApplyDisplayValues() override;
	virtual void ResetValueToDefault() override;
	virtual bool IsValueAtDefault() const override;
	virtual void BuildDefaultWidgetTree() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> Array_VerticalBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> AddButt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Array")
	TSubclassOf<URuntimeGameplaySettingsArrayElementWidgetBase> ElementWidgetClass;

	UPROPERTY(Transient)
	TArray<TObjectPtr<URuntimeGameplaySettingsArrayElementWidgetBase>> ElementWidgets;

	void RebuildArrayElements();

	virtual int32 GetArrayValueCount() const;
	virtual void AppendDefaultArrayValue();
	virtual void RemoveArrayValueAt(int32 ElementIndex);
	virtual void ResetArrayValuesToDefault();
	virtual bool IsArrayValueAtDefault() const;
	virtual void SynchronizeArrayValuesFromElementWidgets();
	virtual void InitializeElementWidget(URuntimeGameplaySettingsArrayElementWidgetBase* ElementWidget, int32 ElementIndex);
	virtual TSubclassOf<URuntimeGameplaySettingsArrayElementWidgetBase> GetResolvedElementWidgetClass() const;

private:
	UFUNCTION()
	void HandleAddReleased();

	UFUNCTION()
	void HandleElementDeleteRequested(URuntimeGameplaySettingsArrayElementWidgetBase* ElementWidget);

	UFUNCTION()
	void HandleElementValueChanged(URuntimeGameplaySettingsArrayElementWidgetBase* ElementWidget);

	bool bIsRebuildingArray = false;
};
