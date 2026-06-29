#pragma once

#include "Components/ComboBoxString.h"
#include "UI/Elements/RuntimeGameplaySettingsValueWidgetBase.h"
#include "RuntimeGameplaySettingsEnumWidget.generated.h"

class UTextBlock;
class UEnum;
class UButton;

USTRUCT(BlueprintType)
struct RUNTIMEGAMEPLAYSETTINGS_API FRuntimeGameplaySettingsEnumOption
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Enum")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Enum")
	int32 Value = 0;
};

UCLASS(Blueprintable)
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsEnumWidget : public URuntimeGameplaySettingsValueWidgetBase
{
	GENERATED_BODY()

public:
	URuntimeGameplaySettingsEnumWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Enum")
	void SetOptions(const TArray<FRuntimeGameplaySettingsEnumOption>& InOptions);

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Enum")
	void SetOptionsFromEnum(const UEnum* InEnum, bool bSkipHiddenOptions = true);

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Enum")
	void SetSelectedValue(int32 InValue);

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Enum")
	void SetDefaultSelectedValue(int32 InValue);

	UFUNCTION(BlueprintPure, Category="RuntimeGameplaySettings Enum")
	int32 GetSelectedValue() const;

	UFUNCTION()
	TArray<FString> GetEnumClassPathOptions() const;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void ApplyDisplayValues() override;
	virtual void ResetValueToDefault() override;
	virtual bool IsValueAtDefault() const override;
	virtual void BuildDefaultWidgetTree() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UComboBoxString> Combo_Options;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Enum", meta=(DisplayName="Enum Class", GetOptions="GetEnumClassPathOptions"))
	FString EnumClassPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Enum")
	TArray<FString> AdditionalEnumClassPaths;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Enum")
	bool bSkipHiddenEnumOptions = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Enum")
	TArray<FRuntimeGameplaySettingsEnumOption> Options;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Enum")
	int32 DefaultValue = 0;

private:
	UFUNCTION()
	void HandleSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	const UEnum* ResolveEnumClass() const;
	void RefreshOptionsFromEnumClass();
	void RebuildOptions();
	const FRuntimeGameplaySettingsEnumOption* FindOptionByValue(int32 InValue) const;
	const FRuntimeGameplaySettingsEnumOption* FindOptionByDisplayName(const FString& InDisplayName) const;
	int32 GetDefaultSelectedValue() const;

	int32 CurrentValue = 0;
	int32 RuntimeDefaultValue = 0;
	bool bHasInitializedValue = false;
	bool bHasRuntimeDefaultValue = false;
};
