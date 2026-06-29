#pragma once

#include "Blueprint/UserWidget.h"
#include "RuntimeGameplaySettingsArrayElementWidgetBase.generated.h"

class UButton;
class URuntimeGameplaySettingsArrayElementWidgetBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FRuntimeGameplaySettingsArrayElementDeleteRequestedSignature,
	URuntimeGameplaySettingsArrayElementWidgetBase*,
	ElementWidget);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FRuntimeGameplaySettingsArrayElementValueChangedSignature,
	URuntimeGameplaySettingsArrayElementWidgetBase*,
	ElementWidget);

UCLASS(Abstract)
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsArrayElementWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="RuntimeGameplaySettings Array")
	FRuntimeGameplaySettingsArrayElementDeleteRequestedSignature OnRuntimeGameplaySettingsArrayElementDeleteRequested;

	UPROPERTY(BlueprintAssignable, Category="RuntimeGameplaySettings Array")
	FRuntimeGameplaySettingsArrayElementValueChangedSignature OnRuntimeGameplaySettingsArrayElementValueChanged;

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Array")
	void SetElementIndex(int32 InElementIndex);

	UFUNCTION(BlueprintPure, Category="RuntimeGameplaySettings Array")
	int32 GetElementIndex() const;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void BuildDefaultWidgetTree();
	void EnsureDefaultWidgetTree();

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> DeleteButt;

	void NotifyElementValueChanged();

private:
	UFUNCTION()
	void HandleDeleteReleased();

	int32 ElementIndex = INDEX_NONE;
};
