#pragma once

#include "Blueprint/UserWidget.h"
#include "RuntimeGameplaySettingsButtonWidget.generated.h"

class UButton;
class UTextBlock;
class URuntimeGameplaySettingsButtonWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FRuntimeGameplaySettingsButtonReleasedSignature,
	URuntimeGameplaySettingsButtonWidget*,
	ButtonWidget);

UCLASS(Blueprintable)
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="RuntimeGameplaySettings Button")
	FRuntimeGameplaySettingsButtonReleasedSignature OnRuntimeGameplaySettingsButtonReleased;

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Button")
	void SetButtonText(const FText& InButtonText);

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Button")
	void SetButtonTint(const FLinearColor& InButtonTint);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ButtonText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Button")
	FText Text = INVTEXT("Button");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Button")
	FLinearColor ButtonTint = FLinearColor(0.083333f, 0.083333f, 0.083333f, 1.0f);

private:
	UFUNCTION()
	void HandleButtonReleased();

	void EnsureDefaultWidgetTree();
	void BuildDefaultWidgetTree();
	void ApplyDisplayValues();
	FLinearColor BuildPressedTint() const;
};
