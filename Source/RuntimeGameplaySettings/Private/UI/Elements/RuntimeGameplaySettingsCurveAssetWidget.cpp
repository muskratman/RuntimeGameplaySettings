#include "UI/Elements/RuntimeGameplaySettingsCurveAssetWidget.h"

#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ComboBoxString.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Modules/ModuleManager.h"
#include "Templates/UnrealTemplate.h"
#include "UI/Elements/RuntimeGameplaySettingsElementWidgetHelpers.h"

namespace
{
	constexpr const TCHAR* NoneOptionName = TEXT("None");

	FName NormalizePackageRootPath(const FName& InPath)
	{
		FString PathString = InPath.ToString();
		PathString.TrimStartAndEndInline();

		while (PathString.Len() > 1 && PathString.EndsWith(TEXT("/")))
		{
			PathString.LeftChopInline(1);
		}

		return PathString.IsEmpty() ? NAME_None : FName(*PathString);
	}
}

URuntimeGameplaySettingsCurveAssetWidget::URuntimeGameplaySettingsCurveAssetWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AllowedRootPaths.Add(TEXT("/Game/_FortySouls"));
	AllowedRootPaths.Add(TEXT("/Game/Data"));
}

void URuntimeGameplaySettingsCurveAssetWidget::SetAllowedRootPaths(const TArray<FName>& InAllowedRootPaths)
{
	AllowedRootPaths = InAllowedRootPaths;
	NormalizeAllowedRootPaths();
	RebuildAssetOptions();

	if (!CurrentCurveAssetPath.IsNull() && !IsPathUnderAllowedRoots(CurrentCurveAssetPath.ToString()))
	{
		CurrentCurveAssetPath = FSoftObjectPath();
	}

	ApplyDisplayValues();
}

void URuntimeGameplaySettingsCurveAssetWidget::RefreshCurveAssets()
{
	NormalizeAllowedRootPaths();
	RebuildAssetOptions();
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsCurveAssetWidget::SetCurveAssetPath(const FSoftObjectPath& InCurveAssetPath)
{
	if (InCurveAssetPath.IsNull() || IsPathUnderAllowedRoots(InCurveAssetPath.ToString()))
	{
		CurrentCurveAssetPath = InCurveAssetPath;
	}
	else
	{
		CurrentCurveAssetPath = FSoftObjectPath();
	}

	bHasInitializedCurveAssetPath = true;
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsCurveAssetWidget::SetDefaultCurveAssetPath(const FSoftObjectPath& InCurveAssetPath)
{
	if (InCurveAssetPath.IsNull() || IsPathUnderAllowedRoots(InCurveAssetPath.ToString()))
	{
		RuntimeDefaultCurveAssetPath = InCurveAssetPath;
	}
	else
	{
		RuntimeDefaultCurveAssetPath = FSoftObjectPath();
	}

	bHasRuntimeDefaultCurveAssetPath = true;
	UpdateResetButtonState();
}

FSoftObjectPath URuntimeGameplaySettingsCurveAssetWidget::GetCurveAssetPath() const
{
	if (Combo_CurveAssets)
	{
		if (const FSoftObjectPath* SelectedPath = DisplayNameToAssetPath.Find(Combo_CurveAssets->GetSelectedOption()))
		{
			return *SelectedPath;
		}
	}

	return CurrentCurveAssetPath;
}

UCurveFloat* URuntimeGameplaySettingsCurveAssetWidget::LoadSelectedCurveAsset() const
{
	const FSoftObjectPath SelectedPath = GetCurveAssetPath();
	if (SelectedPath.IsNull() || !IsPathUnderAllowedRoots(SelectedPath.ToString()))
	{
		return nullptr;
	}

	if (UObject* ExistingObject = SelectedPath.ResolveObject())
	{
		return Cast<UCurveFloat>(ExistingObject);
	}

	return Cast<UCurveFloat>(SelectedPath.TryLoad());
}

FRuntimeFloatCurve URuntimeGameplaySettingsCurveAssetWidget::GetCurveValue() const
{
	FRuntimeFloatCurve CurveValue;
	if (UCurveFloat* CurveAsset = LoadSelectedCurveAsset())
	{
		CurveValue.EditorCurveData = CurveAsset->FloatCurve;
		CurveValue.ExternalCurve = CurveAsset;
	}

	return CurveValue;
}

void URuntimeGameplaySettingsCurveAssetWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!bHasInitializedCurveAssetPath)
	{
		CurrentCurveAssetPath = DefaultCurveAssetPath;
	}
	if (!bHasRuntimeDefaultCurveAssetPath)
	{
		RuntimeDefaultCurveAssetPath = DefaultCurveAssetPath;
	}

	NormalizeAllowedRootPaths();
	RebuildAssetOptions();
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsCurveAssetWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Combo_CurveAssets)
	{
		Combo_CurveAssets->OnSelectionChanged.AddDynamic(this, &URuntimeGameplaySettingsCurveAssetWidget::HandleSelectionChanged);
	}
	CurrentCurveAssetPath = DefaultCurveAssetPath;
	bHasInitializedCurveAssetPath = true;
	if (!bHasRuntimeDefaultCurveAssetPath)
	{
		RuntimeDefaultCurveAssetPath = DefaultCurveAssetPath;
	}

	NormalizeAllowedRootPaths();
	RebuildAssetOptions();
	ApplyDisplayValues();
}

void URuntimeGameplaySettingsCurveAssetWidget::ResetValueToDefault()
{
	SetCurveAssetPath(GetDefaultCurveAssetPath());
}

bool URuntimeGameplaySettingsCurveAssetWidget::IsValueAtDefault() const
{
	return GetCurveAssetPath() == GetDefaultCurveAssetPath();
}

void URuntimeGameplaySettingsCurveAssetWidget::BuildDefaultWidgetTree()
{
	using namespace RuntimeGameplaySettingsElementWidgetHelpers;

	if (!WidgetTree)
	{
		return;
	}

	UHorizontalBox* RootBox = ConstructWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_2"));
	Txt_Name = CreateTextBlock(WidgetTree, TEXT("Txt_Name"), INVTEXT("Curve Asset"), LabelFontSize);
	Combo_CurveAssets = ConstructWidget<UComboBoxString>(WidgetTree, TEXT("Combo_CurveAssets"));
	ResetButtonWidget = CreateResetButtonWidget(WidgetTree);

	if (!RootBox)
	{
		return;
	}

	if (UHorizontalBoxSlot* NameSlot = RootBox->AddChildToHorizontalBox(WrapLabelInSizeBox(WidgetTree, Txt_Name)))
	{
		NameSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		NameSlot->SetPadding(FMargin(0.0f));
		NameSlot->SetHorizontalAlignment(HAlign_Left);
		NameSlot->SetVerticalAlignment(VAlign_Center);
	}
	AddToHorizontalBox(
		RootBox,
		WrapInSizeBox(WidgetTree, TEXT("SizeBox_1"), Combo_CurveAssets, ValueWidth),
		FMargin(0.0f),
		HAlign_Left);
	AddToHorizontalBox(RootBox, ResetButtonWidget, FMargin(10.0f, 0.0f, 0.0f, 0.0f), HAlign_Right);

	WidgetTree->RootWidget = RootBox;
}

void URuntimeGameplaySettingsCurveAssetWidget::HandleSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	(void)SelectionType;

	if (bIsSynchronizing)
	{
		return;
	}

	if (const FSoftObjectPath* SelectedPath = DisplayNameToAssetPath.Find(SelectedItem))
	{
		CurrentCurveAssetPath = *SelectedPath;
		bHasInitializedCurveAssetPath = true;
	}

	ApplyDisplayValues();
}

void URuntimeGameplaySettingsCurveAssetWidget::ApplyDisplayValues()
{
	TGuardValue<bool> SynchronizationGuard(bIsSynchronizing, true);

	if (Combo_CurveAssets)
	{
		const FSoftObjectPath SelectedPath = GetCurveAssetPath();
		const FString SelectedOption = SelectedPath.IsNull() ? FString(NoneOptionName) : FindDisplayNameByPath(SelectedPath);

		if (!SelectedOption.IsEmpty())
		{
			Combo_CurveAssets->SetSelectedOption(SelectedOption);
		}
		else
		{
			CurrentCurveAssetPath = FSoftObjectPath();
			Combo_CurveAssets->SetSelectedOption(NoneOptionName);
		}
	}

	Super::ApplyDisplayValues();
}

void URuntimeGameplaySettingsCurveAssetWidget::RebuildAssetOptions()
{
	CachedCurveAssetPaths.Reset();
	DisplayNameToAssetPath.Reset();
	DisplayNameToAssetPath.Add(NoneOptionName, FSoftObjectPath());

	TArray<FString> PathStrings;
	FARFilter Filter;
	Filter.ClassPaths.Add(UCurveFloat::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = true;

	for (const FName RootPath : AllowedRootPaths)
	{
		if (!RootPath.IsNone())
		{
			Filter.PackagePaths.Add(RootPath);
			PathStrings.Add(RootPath.ToString());
		}
	}

	TGuardValue<bool> SynchronizationGuard(bIsSynchronizing, true);
	if (Combo_CurveAssets)
	{
		Combo_CurveAssets->ClearOptions();
		Combo_CurveAssets->AddOption(NoneOptionName);
	}

	if (PathStrings.IsEmpty())
	{
		return;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

#if WITH_EDITOR
	AssetRegistry.ScanPathsSynchronous(PathStrings, false, true);
#endif

	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssets(Filter, AssetDataList);
	AssetDataList.Sort([](const FAssetData& LeftAsset, const FAssetData& RightAsset)
	{
		if (LeftAsset.PackagePath == RightAsset.PackagePath)
		{
			return LeftAsset.AssetName.LexicalLess(RightAsset.AssetName);
		}

		return LeftAsset.PackagePath.LexicalLess(RightAsset.PackagePath);
	});

	for (const FAssetData& AssetData : AssetDataList)
	{
		if (!AssetData.IsValid())
		{
			continue;
		}

		const FSoftObjectPath CurveAssetPath = AssetData.GetSoftObjectPath();
		FString DisplayName = BuildDisplayName(AssetData);
		FString UniqueDisplayName = DisplayName;
		int32 DuplicateIndex = 2;
		while (DisplayNameToAssetPath.Contains(UniqueDisplayName))
		{
			UniqueDisplayName = FString::Printf(TEXT("%s [%d]"), *DisplayName, DuplicateIndex++);
		}

		CachedCurveAssetPaths.Add(CurveAssetPath);
		DisplayNameToAssetPath.Add(UniqueDisplayName, CurveAssetPath);

		if (Combo_CurveAssets)
		{
			Combo_CurveAssets->AddOption(UniqueDisplayName);
		}
	}
}

void URuntimeGameplaySettingsCurveAssetWidget::NormalizeAllowedRootPaths()
{
	for (FName& RootPath : AllowedRootPaths)
	{
		RootPath = NormalizePackageRootPath(RootPath);
	}

	AllowedRootPaths.RemoveAll([](const FName& RootPath)
	{
		return RootPath.IsNone();
	});
}

bool URuntimeGameplaySettingsCurveAssetWidget::IsPathUnderAllowedRoots(const FString& InObjectPath) const
{
	for (const FName RootPath : AllowedRootPaths)
	{
		const FString RootPathString = RootPath.ToString();
		const FString RootPathPrefix = RootPathString.EndsWith(TEXT("/"))
			? RootPathString
			: RootPathString + TEXT("/");

		if (InObjectPath.Equals(RootPathString, ESearchCase::IgnoreCase)
			|| InObjectPath.StartsWith(RootPathPrefix, ESearchCase::IgnoreCase))
		{
			return true;
		}
	}

	return false;
}

FString URuntimeGameplaySettingsCurveAssetWidget::BuildDisplayName(const FAssetData& InAssetData) const
{
	return FString::Printf(
		TEXT("%s\n(%s)"),
		*InAssetData.AssetName.ToString(),
		*InAssetData.GetSoftObjectPath().ToString());
}

FString URuntimeGameplaySettingsCurveAssetWidget::FindDisplayNameByPath(const FSoftObjectPath& InCurveAssetPath) const
{
	for (const TPair<FString, FSoftObjectPath>& OptionPair : DisplayNameToAssetPath)
	{
		if (OptionPair.Value == InCurveAssetPath)
		{
			return OptionPair.Key;
		}
	}

	return FString();
}

FSoftObjectPath URuntimeGameplaySettingsCurveAssetWidget::GetDefaultCurveAssetPath() const
{
	return bHasRuntimeDefaultCurveAssetPath ? RuntimeDefaultCurveAssetPath : DefaultCurveAssetPath;
}
