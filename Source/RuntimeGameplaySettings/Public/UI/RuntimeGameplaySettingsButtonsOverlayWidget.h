#pragma once

#include "Blueprint/UserWidget.h"
#include "RuntimeGameplaySettingsButtonsOverlayWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRuntimeGameplaySettingsButtonsOverlayEvent);

UCLASS(Blueprintable)
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsButtonsOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="RuntimeGameplaySettings Buttons")
	FRuntimeGameplaySettingsButtonsOverlayEvent OnShowSaveAsRequested;

	UPROPERTY(BlueprintAssignable, Category="RuntimeGameplaySettings Buttons")
	FRuntimeGameplaySettingsButtonsOverlayEvent OnSaveRequested;

	UPROPERTY(BlueprintAssignable, Category="RuntimeGameplaySettings Buttons")
	FRuntimeGameplaySettingsButtonsOverlayEvent OnCloseRequested;

	UPROPERTY(BlueprintAssignable, Category="RuntimeGameplaySettings Buttons")
	FRuntimeGameplaySettingsButtonsOverlayEvent OnResetAllToDefaultRequested;

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Buttons")
	void SetCurrentSlotName(const FString& SlotName);

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Buttons")
	void SetSaveEnabled(bool bCanSave);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Txt_CurrSlotName;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Butt_ShowSaveAs;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Butt_Save;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Butt_Close;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Butt_ResetAllToDefault;

private:
	UFUNCTION()
	void HandleShowSaveAsClicked();

	UFUNCTION()
	void HandleSaveClicked();

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleResetAllToDefaultClicked();

	void EnsureDefaultWidgetTree();
	void BuildDefaultWidgetTree();
};
