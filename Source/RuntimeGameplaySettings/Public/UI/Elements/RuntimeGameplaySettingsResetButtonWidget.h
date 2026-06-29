#pragma once

#include "Blueprint/UserWidget.h"
#include "RuntimeGameplaySettingsResetButtonWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRuntimeGameplaySettingsResetButtonReleasedSignature);

UCLASS(Blueprintable)
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsResetButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="RuntimeGameplaySettings Reset")
	FRuntimeGameplaySettingsResetButtonReleasedSignature OnRuntimeGameplaySettingsResetButtonReleased;

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Reset")
	void SetResetEnabled(bool bInIsEnabled);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> ResetButt;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ResetText;

private:
	UFUNCTION()
	void HandleResetReleased();

	void EnsureDefaultWidgetTree();
	void BuildDefaultWidgetTree();
	void ApplyDisplayValues();
};
