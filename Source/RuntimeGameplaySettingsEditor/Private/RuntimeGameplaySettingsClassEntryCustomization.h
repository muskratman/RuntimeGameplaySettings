#pragma once

#include "Data/RuntimeGameplaySettingsTypes.h"
#include "IPropertyTypeCustomization.h"

class IPropertyHandle;
class IPropertyUtilities;
class SVerticalBox;

class FRuntimeGameplaySettingsClassEntryCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual void CustomizeHeader(
		TSharedRef<IPropertyHandle> StructPropertyHandle,
		FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	virtual void CustomizeChildren(
		TSharedRef<IPropertyHandle> StructPropertyHandle,
		IDetailChildrenBuilder& ChildBuilder,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:
	TSharedRef<SWidget> BuildAvailablePropertiesWidget();
	TSharedRef<SWidget> BuildAvailablePropertyRow(const FRuntimeGameplaySettingsDiscoveredProperty& DiscoveredProperty, bool bIsHeader) const;

	void RebuildAvailablePropertiesList();
	void HandleAvailableSearchTextChanged(const FText& InSearchText);
	FReply HandleSetAvailableSortMode(int32 SortMode);

	FText GetHeaderSummaryText() const;
	FText GetAvailableHeaderText() const;
	FText GetAvailableSortModeText() const;
	FText GetDiscoveredPropertyLabel(const FRuntimeGameplaySettingsDiscoveredProperty& DiscoveredProperty) const;
	FText GetValueTypeText(ERuntimeGameplaySettingsValueType ValueType) const;
	FRuntimeGameplaySettingsClassEntry* GetMutableClassEntry() const;
	const FRuntimeGameplaySettingsClassEntry* GetClassEntry() const;

	TSharedPtr<IPropertyHandle> ClassEntryHandle;
	TSharedPtr<IPropertyUtilities> PropertyUtilities;
	TSharedPtr<SVerticalBox> AvailablePropertiesBox;
	FString AvailableSearchString;
	int32 AvailableSortMode = 0;
};
