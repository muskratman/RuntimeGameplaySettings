#pragma once

#include "Data/RuntimeGameplaySettingsTypes.h"
#include "IPropertyTypeCustomization.h"
#include "Types/SlateEnums.h"

class IPropertyHandle;
class IPropertyUtilities;
struct FEdGraphSchemaAction;
struct FGraphActionListBuilderBase;

class FRuntimeGameplaySettingsPropertyEntryCustomization : public IPropertyTypeCustomization
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
	TSharedRef<SWidget> BuildComponentPickerMenuContent();
	TSharedRef<SWidget> BuildPropertyPickerMenuContent();

	void CollectComponentActions(FGraphActionListBuilderBase& OutAllActions);
	void CollectPropertyActions(FGraphActionListBuilderBase& OutAllActions);
	void HandleComponentActionSelected(const TArray<TSharedPtr<FEdGraphSchemaAction>>& SelectedActions, ESelectInfo::Type InSelectionType);
	void HandlePropertyActionSelected(const TArray<TSharedPtr<FEdGraphSchemaAction>>& SelectedActions, ESelectInfo::Type InSelectionType);
	FReply HandleSelectComponent(FName ComponentName, FName ComponentClassName, const FString& ComponentClassPath);
	FReply HandleSelectProperty(int32 AvailablePropertyIndex);

	FText GetSelectedComponentLabel() const;
	FText GetSelectedPropertyLabel() const;
	FText GetCurrentValueTypeText() const;
	FText GetRuntimeCategoryText() const;
	FText GetSubCategoryText() const;
	FText GetDiscoveredPropertyLabel(const FRuntimeGameplaySettingsDiscoveredProperty& DiscoveredProperty) const;
	FText GetValueTypeText(ERuntimeGameplaySettingsValueType ValueType) const;
	void HandleRuntimeCategoryCommitted(const FText& InText, ETextCommit::Type CommitType);
	void HandleSubCategoryCommitted(const FText& InText, ETextCommit::Type CommitType);
	bool HasSelectedComponent() const;
	bool IsPropertyInSelectedComponent(const FRuntimeGameplaySettingsDiscoveredProperty& DiscoveredProperty) const;
	bool IsPropertySelectedByOtherRow(const FRuntimeGameplaySettingsDiscoveredProperty& DiscoveredProperty) const;
	bool SetPropertyEntryHandleValue(TSharedRef<IPropertyHandle> InPropertyEntryHandle, const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry) const;
	int32 FindCurrentPropertyIndex() const;
	FRuntimeGameplaySettingsPropertyEntry* GetMutablePropertyEntry() const;
	const FRuntimeGameplaySettingsPropertyEntry* GetPropertyEntry() const;
	FRuntimeGameplaySettingsClassEntry* GetMutableOwnerClassEntry() const;
	const FRuntimeGameplaySettingsClassEntry* GetOwnerClassEntry() const;

	TSharedPtr<IPropertyHandle> PropertyEntryHandle;
	TSharedPtr<IPropertyUtilities> PropertyUtilities;
};
