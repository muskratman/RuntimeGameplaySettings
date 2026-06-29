#pragma once

#include "Components/EditableTextBox.h"
#include "UI/Elements/RuntimeGameplaySettingsValueWidgetBase.h"
#include "RuntimeGameplaySettingsVectorWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS(Blueprintable)
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsVectorWidget : public URuntimeGameplaySettingsValueWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Vector")
	void SetVectorValue(const FVector& InVectorValue);

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Vector")
	void SetDefaultVectorValue(const FVector& InVectorValue);

	UFUNCTION(BlueprintPure, Category="RuntimeGameplaySettings Vector")
	FVector GetVectorValue() const;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void ApplyDisplayValues() override;
	virtual void ResetValueToDefault() override;
	virtual bool IsValueAtDefault() const override;
	virtual void BuildDefaultWidgetTree() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> Editable_X;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> Editable_Y;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> Editable_Z;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Vector")
	float X = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Vector")
	float Y = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Vector")
	float Z = 0.0f;

private:
	UFUNCTION()
	void HandleXTextCommitted(const FText& InText, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void HandleYTextCommitted(const FText& InText, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void HandleZTextCommitted(const FText& InText, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void HandleVectorTextChanged(const FText& InText);

	void CacheDefaultVectorValueIfNeeded();
	FVector GetDefaultVectorValue() const;

	FVector RuntimeDefaultValue = FVector::ZeroVector;
	bool bHasRuntimeDefaultValue = false;
};
