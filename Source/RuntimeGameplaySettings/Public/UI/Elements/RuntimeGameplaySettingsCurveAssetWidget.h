#pragma once

#include "Components/ComboBoxString.h"
#include "Curves/CurveFloat.h"
#include "UI/Elements/RuntimeGameplaySettingsValueWidgetBase.h"
#include "UObject/SoftObjectPath.h"
#include "RuntimeGameplaySettingsCurveAssetWidget.generated.h"

class UTextBlock;
class UButton;
struct FAssetData;

UCLASS(Blueprintable)
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsCurveAssetWidget : public URuntimeGameplaySettingsValueWidgetBase
{
	GENERATED_BODY()

public:
	URuntimeGameplaySettingsCurveAssetWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Curve Asset")
	void SetAllowedRootPaths(const TArray<FName>& InAllowedRootPaths);

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Curve Asset")
	void RefreshCurveAssets();

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Curve Asset")
	void SetCurveAssetPath(const FSoftObjectPath& InCurveAssetPath);

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Curve Asset")
	void SetDefaultCurveAssetPath(const FSoftObjectPath& InCurveAssetPath);

	UFUNCTION(BlueprintPure, Category="RuntimeGameplaySettings Curve Asset")
	FSoftObjectPath GetCurveAssetPath() const;

	UFUNCTION(BlueprintCallable, Category="RuntimeGameplaySettings Curve Asset")
	UCurveFloat* LoadSelectedCurveAsset() const;

	UFUNCTION(BlueprintPure, Category="RuntimeGameplaySettings Curve Asset")
	FRuntimeFloatCurve GetCurveValue() const;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void ApplyDisplayValues() override;
	virtual void ResetValueToDefault() override;
	virtual bool IsValueAtDefault() const override;
	virtual void BuildDefaultWidgetTree() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UComboBoxString> Combo_CurveAssets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Curve Asset")
	TArray<FName> AllowedRootPaths;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RuntimeGameplaySettings Curve Asset")
	FSoftObjectPath DefaultCurveAssetPath;

private:
	UFUNCTION()
	void HandleSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	void RebuildAssetOptions();
	void NormalizeAllowedRootPaths();
	bool IsPathUnderAllowedRoots(const FString& InObjectPath) const;
	FString BuildDisplayName(const FAssetData& InAssetData) const;
	FString FindDisplayNameByPath(const FSoftObjectPath& InCurveAssetPath) const;
	FSoftObjectPath GetDefaultCurveAssetPath() const;

	TArray<FSoftObjectPath> CachedCurveAssetPaths;
	TMap<FString, FSoftObjectPath> DisplayNameToAssetPath;
	FSoftObjectPath CurrentCurveAssetPath;
	FSoftObjectPath RuntimeDefaultCurveAssetPath;
	bool bHasInitializedCurveAssetPath = false;
	bool bHasRuntimeDefaultCurveAssetPath = false;
};
