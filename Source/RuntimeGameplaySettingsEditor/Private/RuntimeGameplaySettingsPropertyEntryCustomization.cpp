#include "RuntimeGameplaySettingsPropertyEntryCustomization.h"

#include "DetailWidgetRow.h"
#include "EdGraph/EdGraphSchema.h"
#include "Framework/Application/SlateApplication.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "PropertyHandle.h"
#include "Runtime/RuntimeGameplaySettingsPropertyAccess.h"
#include "SGraphActionMenu.h"
#include "ScopedTransaction.h"
#include "Styling/AppStyle.h"
#include "UObject/UnrealType.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FRuntimeGameplaySettingsPropertyEntryCustomization"

struct FRuntimeGameplaySettingsComponentSchemaAction : public FEdGraphSchemaAction
{
	static FName StaticGetTypeId()
	{
		static FName Type(TEXT("FRuntimeGameplaySettingsComponentSchemaAction"));
		return Type;
	}

	virtual FName GetTypeId() const override
	{
		return StaticGetTypeId();
	}

	FRuntimeGameplaySettingsComponentSchemaAction(
		FName InComponentName,
		FName InComponentClassName,
		FString InComponentClassPath,
		FText InCategory,
		FText InMenuDescription,
		FText InToolTip,
		FText InKeywords)
		: FEdGraphSchemaAction(
			MoveTemp(InCategory),
			MoveTemp(InMenuDescription),
			MoveTemp(InToolTip),
			0,
			MoveTemp(InKeywords))
		, ComponentName(InComponentName)
		, ComponentClassName(InComponentClassName)
		, ComponentClassPath(MoveTemp(InComponentClassPath))
	{
	}

	FName ComponentName;
	FName ComponentClassName;
	FString ComponentClassPath;
};

struct FRuntimeGameplaySettingsPropertySchemaAction : public FEdGraphSchemaAction
{
	static FName StaticGetTypeId()
	{
		static FName Type(TEXT("FRuntimeGameplaySettingsPropertySchemaAction"));
		return Type;
	}

	virtual FName GetTypeId() const override
	{
		return StaticGetTypeId();
	}

	FRuntimeGameplaySettingsPropertySchemaAction(
		int32 InAvailablePropertyIndex,
		FText InCategory,
		FText InMenuDescription,
		FText InToolTip,
		FText InKeywords)
		: FEdGraphSchemaAction(
			MoveTemp(InCategory),
			MoveTemp(InMenuDescription),
			MoveTemp(InToolTip),
			0,
			MoveTemp(InKeywords))
		, AvailablePropertyIndex(InAvailablePropertyIndex)
	{
	}

	int32 AvailablePropertyIndex = INDEX_NONE;
};

TSharedRef<IPropertyTypeCustomization> FRuntimeGameplaySettingsPropertyEntryCustomization::MakeInstance()
{
	return MakeShared<FRuntimeGameplaySettingsPropertyEntryCustomization>();
}

void FRuntimeGameplaySettingsPropertyEntryCustomization::CustomizeHeader(
	TSharedRef<IPropertyHandle> StructPropertyHandle,
	FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	PropertyEntryHandle = StructPropertyHandle;
	PropertyUtilities = CustomizationUtils.GetPropertyUtilities();

	HeaderRow
		.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(620.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(180.0f)
				[
					SNew(SComboButton)
					.ContentPadding(FMargin(4.0f, 2.0f))
					.OnGetMenuContent(this, &FRuntimeGameplaySettingsPropertyEntryCustomization::BuildComponentPickerMenuContent)
					.ButtonContent()
					[
						SNew(STextBlock)
						.Text(this, &FRuntimeGameplaySettingsPropertyEntryCustomization::GetSelectedComponentLabel)
						.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
					]
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SComboButton)
				.ContentPadding(FMargin(4.0f, 2.0f))
				.OnGetMenuContent(this, &FRuntimeGameplaySettingsPropertyEntryCustomization::BuildPropertyPickerMenuContent)
				.IsEnabled(this, &FRuntimeGameplaySettingsPropertyEntryCustomization::HasSelectedComponent)
				.ButtonContent()
				[
					SNew(STextBlock)
					.Text(this, &FRuntimeGameplaySettingsPropertyEntryCustomization::GetSelectedPropertyLabel)
					.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 10.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(this, &FRuntimeGameplaySettingsPropertyEntryCustomization::GetCurrentValueTypeText)
				.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 10.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(INVTEXT("|"))
				.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SEditableTextBox)
				.MinDesiredWidth(160.0f)
				.Text(this, &FRuntimeGameplaySettingsPropertyEntryCustomization::GetRuntimeCategoryText)
				.HintText(LOCTEXT("RuntimeCategoryHint", "Widget category"))
				.OnTextCommitted(this, &FRuntimeGameplaySettingsPropertyEntryCustomization::HandleRuntimeCategoryCommitted)
				.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
			]
		];
}

void FRuntimeGameplaySettingsPropertyEntryCustomization::CustomizeChildren(
	TSharedRef<IPropertyHandle> StructPropertyHandle,
	IDetailChildrenBuilder& ChildBuilder,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	PropertyEntryHandle = StructPropertyHandle;
	PropertyUtilities = CustomizationUtils.GetPropertyUtilities();
}

TSharedRef<SWidget> FRuntimeGameplaySettingsPropertyEntryCustomization::BuildComponentPickerMenuContent()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Menu.Background"))
		.Padding(5.0f)
		[
			SNew(SBox)
			.WidthOverride(360.0f)
			.HeightOverride(360.0f)
			[
				SNew(SGraphActionMenu)
				.OnCollectAllActions(this, &FRuntimeGameplaySettingsPropertyEntryCustomization::CollectComponentActions)
				.OnActionSelected(this, &FRuntimeGameplaySettingsPropertyEntryCustomization::HandleComponentActionSelected)
				.AutoExpandActionMenu(false)
				.ShowFilterTextBox(true)
				.AlphaSortItems(true)
				.SortItemsRecursively(true)
			]
		];
}

TSharedRef<SWidget> FRuntimeGameplaySettingsPropertyEntryCustomization::BuildPropertyPickerMenuContent()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Menu.Background"))
		.Padding(5.0f)
		[
			SNew(SBox)
			.WidthOverride(460.0f)
			.HeightOverride(420.0f)
			[
				SNew(SGraphActionMenu)
				.OnCollectAllActions(this, &FRuntimeGameplaySettingsPropertyEntryCustomization::CollectPropertyActions)
				.OnActionSelected(this, &FRuntimeGameplaySettingsPropertyEntryCustomization::HandlePropertyActionSelected)
				.AutoExpandActionMenu(false)
				.ShowFilterTextBox(true)
				.AlphaSortItems(true)
				.SortItemsRecursively(true)
			]
		];
}

void FRuntimeGameplaySettingsPropertyEntryCustomization::CollectComponentActions(
	FGraphActionListBuilderBase& OutAllActions)
{
	const FRuntimeGameplaySettingsClassEntry* ClassEntry = GetOwnerClassEntry();
	if (!ClassEntry)
	{
		return;
	}

	TSet<FString> AddedComponentKeys;
	for (const FRuntimeGameplaySettingsDiscoveredProperty& DiscoveredProperty : ClassEntry->AvailableProperties)
	{
		const FString ComponentKey = FString::Printf(
			TEXT("%s|%s"),
			*DiscoveredProperty.ComponentName.ToString(),
			*DiscoveredProperty.ComponentClassPath);
		if (AddedComponentKeys.Contains(ComponentKey))
		{
			continue;
		}

		AddedComponentKeys.Add(ComponentKey);

		const bool bIsActor = DiscoveredProperty.ComponentName.IsNone();
		const FText CategoryText = bIsActor
			? LOCTEXT("ActorComponentPickerCategory", "Actor")
			: LOCTEXT("ComponentPickerCategory", "Components");
		const FText MenuText = bIsActor
			? LOCTEXT("ActorComponentPickerItem", "Actor")
			: FText::FromName(DiscoveredProperty.ComponentName);
		const FText ClassText = DiscoveredProperty.ComponentClassName.IsNone()
			? INVTEXT("")
			: FText::FromName(DiscoveredProperty.ComponentClassName);
		const FText ToolTipText = bIsActor
			? LOCTEXT("ActorComponentPickerTooltip", "Actor-level properties")
			: FText::Format(
				LOCTEXT("ComponentPickerTooltip", "Component: {0}\nClass: {1}"),
				FText::FromName(DiscoveredProperty.ComponentName),
				ClassText);
		const FString ComponentNameString = bIsActor
			? FString(TEXT("Actor"))
			: DiscoveredProperty.ComponentName.ToString();
		const FText KeywordsText = FText::FromString(FString::Printf(
			TEXT("%s %s %s %s"),
			*ComponentNameString,
			*DiscoveredProperty.ComponentClassName.ToString(),
			*DiscoveredProperty.ComponentClassPath,
			bIsActor ? TEXT("TargetClass") : TEXT("Component")));

		OutAllActions.AddAction(MakeShared<FRuntimeGameplaySettingsComponentSchemaAction>(
			DiscoveredProperty.ComponentName,
			DiscoveredProperty.ComponentClassName,
			DiscoveredProperty.ComponentClassPath,
			CategoryText,
			MenuText,
			ToolTipText,
			KeywordsText));
	}
}

void FRuntimeGameplaySettingsPropertyEntryCustomization::CollectPropertyActions(
	FGraphActionListBuilderBase& OutAllActions)
{
	const FRuntimeGameplaySettingsClassEntry* ClassEntry = GetOwnerClassEntry();
	if (!ClassEntry)
	{
		return;
	}

	for (int32 AvailableIndex = 0; AvailableIndex < ClassEntry->AvailableProperties.Num(); ++AvailableIndex)
	{
		const FRuntimeGameplaySettingsDiscoveredProperty& DiscoveredProperty =
			ClassEntry->AvailableProperties[AvailableIndex];
		if (!IsPropertyInSelectedComponent(DiscoveredProperty)
			|| IsPropertySelectedByOtherRow(DiscoveredProperty))
		{
			continue;
		}

		const FText CategoryText = DiscoveredProperty.Category.IsNone()
			? LOCTEXT("PropertyPickerDefaultCategory", "Properties")
			: FText::FromName(DiscoveredProperty.Category);
		const FText MenuText = FText::FromName(DiscoveredProperty.PropertyName);
		const FText ValueTypeText = GetValueTypeText(DiscoveredProperty.ValueType);
		const FText DisplayNameText = GetDiscoveredPropertyLabel(DiscoveredProperty);
		const FString PropertyPathString =
			FRuntimeGameplaySettingsPropertyAccess::BuildPropertyPathString(
				DiscoveredProperty.PropertyPath,
				DiscoveredProperty.PropertyName);
		const FText ToolTipText = FText::Format(
			LOCTEXT("AvailablePropertyTooltip", "Component: {0}\nDisplayName: {1}\nPropertyPath: {2}\nType: {3}"),
			DiscoveredProperty.ComponentName.IsNone()
				? LOCTEXT("ActorPropertyComponentTooltip", "Actor")
				: FText::FromName(DiscoveredProperty.ComponentName),
			DisplayNameText,
			FText::FromString(PropertyPathString),
			ValueTypeText);
		const FText KeywordsText = FText::FromString(FString::Printf(
			TEXT("%s %s %s %s %s %s %s"),
			*DisplayNameText.ToString(),
			*PropertyPathString,
			*DiscoveredProperty.ComponentName.ToString(),
			*DiscoveredProperty.ComponentClassName.ToString(),
			*DiscoveredProperty.PropertyName.ToString(),
			*DiscoveredProperty.OwnerClassName.ToString(),
			*ValueTypeText.ToString()));

		OutAllActions.AddAction(MakeShared<FRuntimeGameplaySettingsPropertySchemaAction>(
			AvailableIndex,
			CategoryText,
			MenuText,
			ToolTipText,
			KeywordsText));
	}
}

void FRuntimeGameplaySettingsPropertyEntryCustomization::HandleComponentActionSelected(
	const TArray<TSharedPtr<FEdGraphSchemaAction>>& SelectedActions,
	ESelectInfo::Type InSelectionType)
{
	if (InSelectionType != ESelectInfo::OnMouseClick
		&& InSelectionType != ESelectInfo::OnKeyPress)
	{
		return;
	}

	if (SelectedActions.Num() == 0 || !SelectedActions[0].IsValid())
	{
		return;
	}

	const TSharedPtr<FEdGraphSchemaAction> SelectedAction = SelectedActions[0];
	if (!SelectedAction->IsA(FRuntimeGameplaySettingsComponentSchemaAction::StaticGetTypeId()))
	{
		return;
	}

	const TSharedPtr<FRuntimeGameplaySettingsComponentSchemaAction> ComponentAction =
		StaticCastSharedPtr<FRuntimeGameplaySettingsComponentSchemaAction>(SelectedAction);
	if (ComponentAction.IsValid())
	{
		HandleSelectComponent(
			ComponentAction->ComponentName,
			ComponentAction->ComponentClassName,
			ComponentAction->ComponentClassPath);
		FSlateApplication::Get().DismissAllMenus();
	}
}

void FRuntimeGameplaySettingsPropertyEntryCustomization::HandlePropertyActionSelected(
	const TArray<TSharedPtr<FEdGraphSchemaAction>>& SelectedActions,
	ESelectInfo::Type InSelectionType)
{
	if (InSelectionType != ESelectInfo::OnMouseClick
		&& InSelectionType != ESelectInfo::OnKeyPress)
	{
		return;
	}

	if (SelectedActions.Num() == 0 || !SelectedActions[0].IsValid())
	{
		return;
	}

	const TSharedPtr<FEdGraphSchemaAction> SelectedAction = SelectedActions[0];
	if (!SelectedAction->IsA(FRuntimeGameplaySettingsPropertySchemaAction::StaticGetTypeId()))
	{
		return;
	}

	const TSharedPtr<FRuntimeGameplaySettingsPropertySchemaAction> PropertyAction =
		StaticCastSharedPtr<FRuntimeGameplaySettingsPropertySchemaAction>(SelectedAction);
	if (PropertyAction.IsValid())
	{
		HandleSelectProperty(PropertyAction->AvailablePropertyIndex);
		FSlateApplication::Get().DismissAllMenus();
	}
}

FReply FRuntimeGameplaySettingsPropertyEntryCustomization::HandleSelectComponent(
	FName ComponentName,
	FName ComponentClassName,
	const FString& ComponentClassPath)
{
	FRuntimeGameplaySettingsPropertyEntry NewPropertyEntry;
	if (const FRuntimeGameplaySettingsPropertyEntry* CurrentPropertyEntry = GetPropertyEntry())
	{
		NewPropertyEntry = *CurrentPropertyEntry;
	}

	const bool bHadSameComponent =
		HasSelectedComponent()
		&& NewPropertyEntry.ComponentName == ComponentName
		&& NewPropertyEntry.ComponentClassPath.Equals(ComponentClassPath, ESearchCase::CaseSensitive);

	NewPropertyEntry.bHasComponentSelection = true;
	NewPropertyEntry.bIsComponentProperty = !ComponentName.IsNone();
	NewPropertyEntry.ComponentName = ComponentName;
	NewPropertyEntry.ComponentClassName = ComponentClassName;
	NewPropertyEntry.ComponentClassPath = ComponentClassPath;

	if (!bHadSameComponent)
	{
		NewPropertyEntry.PropertyName = NAME_None;
		NewPropertyEntry.PropertyPath.Reset();
		NewPropertyEntry.OwnerClassName = NAME_None;
		NewPropertyEntry.DisplayName = FText::GetEmpty();
		NewPropertyEntry.Category = NAME_None;
		NewPropertyEntry.ValueType = ERuntimeGameplaySettingsValueType::Unsupported;
		NewPropertyEntry.EnumPath.Reset();
	}

	const FScopedTransaction Transaction(LOCTEXT("SetRuntimeGameplaySettingsComponent", "Set Runtime Gameplay Settings Component"));

	if (!SetPropertyEntryHandleValue(PropertyEntryHandle.ToSharedRef(), NewPropertyEntry))
	{
		if (FRuntimeGameplaySettingsPropertyEntry* MutablePropertyEntry = GetMutablePropertyEntry())
		{
			PropertyEntryHandle->NotifyPreChange();
			*MutablePropertyEntry = NewPropertyEntry;
			PropertyEntryHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
			PropertyEntryHandle->NotifyFinishedChangingProperties();
		}
	}

	if (PropertyUtilities.IsValid())
	{
		PropertyUtilities->RequestRefresh();
	}

	return FReply::Handled();
}

FReply FRuntimeGameplaySettingsPropertyEntryCustomization::HandleSelectProperty(int32 AvailablePropertyIndex)
{
	FRuntimeGameplaySettingsClassEntry* ClassEntry = GetMutableOwnerClassEntry();
	if (!ClassEntry || !ClassEntry->AvailableProperties.IsValidIndex(AvailablePropertyIndex))
	{
		return FReply::Handled();
	}

	const FRuntimeGameplaySettingsDiscoveredProperty& DiscoveredProperty =
		ClassEntry->AvailableProperties[AvailablePropertyIndex];
	if (!IsPropertyInSelectedComponent(DiscoveredProperty)
		|| IsPropertySelectedByOtherRow(DiscoveredProperty))
	{
		return FReply::Handled();
	}

	const FRuntimeGameplaySettingsPropertyEntry PropertyEntry =
		FRuntimeGameplaySettingsPropertyAccess::BuildPropertyEntry(DiscoveredProperty);
	FRuntimeGameplaySettingsPropertyEntry PreservedPropertyEntry = PropertyEntry;
	if (const FRuntimeGameplaySettingsPropertyEntry* CurrentPropertyEntry = GetPropertyEntry())
	{
		PreservedPropertyEntry.RuntimeCategory = CurrentPropertyEntry->RuntimeCategory;
	}
	const FScopedTransaction Transaction(LOCTEXT("SetRuntimeGameplaySettingsProperty", "Set Runtime Gameplay Settings Property"));

	if (!SetPropertyEntryHandleValue(PropertyEntryHandle.ToSharedRef(), PreservedPropertyEntry))
	{
		if (FRuntimeGameplaySettingsPropertyEntry* MutablePropertyEntry = GetMutablePropertyEntry())
		{
			PropertyEntryHandle->NotifyPreChange();
			*MutablePropertyEntry = PreservedPropertyEntry;
			PropertyEntryHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
			PropertyEntryHandle->NotifyFinishedChangingProperties();
		}
	}

	if (PropertyUtilities.IsValid())
	{
		PropertyUtilities->RequestRefresh();
	}

	return FReply::Handled();
}

FText FRuntimeGameplaySettingsPropertyEntryCustomization::GetSelectedPropertyLabel() const
{
	const FRuntimeGameplaySettingsPropertyEntry* PropertyEntry = GetPropertyEntry();
	if (!HasSelectedComponent())
	{
		return LOCTEXT("SelectComponentFirstPropertyLabel", "Select Component First");
	}

	if (!PropertyEntry || PropertyEntry->PropertyName.IsNone())
	{
		return LOCTEXT("SelectPropertyLabel", "Select Property");
	}

	return FText::FromString(FRuntimeGameplaySettingsPropertyAccess::BuildPropertyPathString(
		PropertyEntry->PropertyPath,
		PropertyEntry->PropertyName));
}

FText FRuntimeGameplaySettingsPropertyEntryCustomization::GetSelectedComponentLabel() const
{
	const FRuntimeGameplaySettingsPropertyEntry* PropertyEntry = GetPropertyEntry();
	if (!PropertyEntry || !HasSelectedComponent())
	{
		return LOCTEXT("SelectComponentLabel", "Select Component");
	}

	return PropertyEntry->ComponentName.IsNone()
		? LOCTEXT("SelectedActorPropertyComponent", "Actor")
		: FText::FromName(PropertyEntry->ComponentName);
}

FText FRuntimeGameplaySettingsPropertyEntryCustomization::GetCurrentValueTypeText() const
{
	const FRuntimeGameplaySettingsPropertyEntry* PropertyEntry = GetPropertyEntry();
	return PropertyEntry
		? GetValueTypeText(PropertyEntry->ValueType)
		: INVTEXT("");
}

FText FRuntimeGameplaySettingsPropertyEntryCustomization::GetRuntimeCategoryText() const
{
	const FRuntimeGameplaySettingsPropertyEntry* PropertyEntry = GetPropertyEntry();
	return PropertyEntry
		? FText::FromString(PropertyEntry->RuntimeCategory)
		: FText::GetEmpty();
}

void FRuntimeGameplaySettingsPropertyEntryCustomization::HandleRuntimeCategoryCommitted(
	const FText& InText,
	ETextCommit::Type CommitType)
{
	FRuntimeGameplaySettingsPropertyEntry* PropertyEntry = GetMutablePropertyEntry();
	if (!PropertyEntry)
	{
		return;
	}

	const FString NewCategory = InText.ToString().TrimStartAndEnd();
	if (PropertyEntry->RuntimeCategory == NewCategory)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("SetRuntimeGameplaySettingsRuntimeCategory", "Set Runtime Gameplay Settings Runtime Category"));
	PropertyEntryHandle->NotifyPreChange();
	PropertyEntry->RuntimeCategory = NewCategory;
	PropertyEntryHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
	PropertyEntryHandle->NotifyFinishedChangingProperties();

	if (PropertyUtilities.IsValid())
	{
		PropertyUtilities->RequestRefresh();
	}
}

bool FRuntimeGameplaySettingsPropertyEntryCustomization::HasSelectedComponent() const
{
	const FRuntimeGameplaySettingsPropertyEntry* PropertyEntry = GetPropertyEntry();
	return PropertyEntry
		&& (PropertyEntry->bHasComponentSelection || !PropertyEntry->PropertyName.IsNone());
}

bool FRuntimeGameplaySettingsPropertyEntryCustomization::IsPropertyInSelectedComponent(
	const FRuntimeGameplaySettingsDiscoveredProperty& DiscoveredProperty) const
{
	const FRuntimeGameplaySettingsPropertyEntry* PropertyEntry = GetPropertyEntry();
	if (!PropertyEntry || !HasSelectedComponent())
	{
		return false;
	}

	if (PropertyEntry->ComponentName.IsNone())
	{
		return DiscoveredProperty.ComponentName.IsNone();
	}

	return DiscoveredProperty.ComponentName == PropertyEntry->ComponentName;
}

FText FRuntimeGameplaySettingsPropertyEntryCustomization::GetDiscoveredPropertyLabel(
	const FRuntimeGameplaySettingsDiscoveredProperty& DiscoveredProperty) const
{
	if (!DiscoveredProperty.DisplayName.IsEmpty())
	{
		return DiscoveredProperty.DisplayName;
	}

	if (!DiscoveredProperty.ComponentName.IsNone())
	{
		return FText::Format(
			INVTEXT("{0}.{1}"),
			FText::FromName(DiscoveredProperty.ComponentName),
			FText::FromString(FRuntimeGameplaySettingsPropertyAccess::BuildPropertyPathString(
				DiscoveredProperty.PropertyPath,
				DiscoveredProperty.PropertyName)));
	}

	return FText::FromString(FRuntimeGameplaySettingsPropertyAccess::BuildPropertyPathString(
		DiscoveredProperty.PropertyPath,
		DiscoveredProperty.PropertyName));
}

FText FRuntimeGameplaySettingsPropertyEntryCustomization::GetValueTypeText(
	ERuntimeGameplaySettingsValueType ValueType) const
{
	switch (ValueType)
	{
	case ERuntimeGameplaySettingsValueType::Bool:
		return INVTEXT("bool");
	case ERuntimeGameplaySettingsValueType::Float:
		return INVTEXT("float");
	case ERuntimeGameplaySettingsValueType::Int:
		return INVTEXT("int32");
	case ERuntimeGameplaySettingsValueType::Enum:
		return INVTEXT("enum");
	case ERuntimeGameplaySettingsValueType::Vector:
		return INVTEXT("FVector");
	case ERuntimeGameplaySettingsValueType::Rotator:
		return INVTEXT("FRotator");
	case ERuntimeGameplaySettingsValueType::String:
		return INVTEXT("FString");
	case ERuntimeGameplaySettingsValueType::Name:
		return INVTEXT("FName");
	default:
		return INVTEXT("unsupported");
	}
}

bool FRuntimeGameplaySettingsPropertyEntryCustomization::IsPropertySelectedByOtherRow(
	const FRuntimeGameplaySettingsDiscoveredProperty& DiscoveredProperty) const
{
	const FRuntimeGameplaySettingsClassEntry* ClassEntry = GetOwnerClassEntry();
	if (!ClassEntry)
	{
		return false;
	}

	const int32 CurrentPropertyIndex = FindCurrentPropertyIndex();
	const FRuntimeGameplaySettingsPropertyEntry* CurrentPropertyEntry = GetPropertyEntry();
	for (int32 PropertyIndex = 0; PropertyIndex < ClassEntry->Properties.Num(); ++PropertyIndex)
	{
		const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry = ClassEntry->Properties[PropertyIndex];
		if (PropertyIndex == CurrentPropertyIndex || &PropertyEntry == CurrentPropertyEntry)
		{
			continue;
		}

		const FString SelectedPropertyPath =
			FRuntimeGameplaySettingsPropertyAccess::BuildPropertyPathString(
				PropertyEntry.PropertyPath,
				PropertyEntry.PropertyName);
		const FString DiscoveredPropertyPath =
			FRuntimeGameplaySettingsPropertyAccess::BuildPropertyPathString(
				DiscoveredProperty.PropertyPath,
				DiscoveredProperty.PropertyName);
		if (SelectedPropertyPath == DiscoveredPropertyPath
			&& PropertyEntry.ComponentName == DiscoveredProperty.ComponentName)
		{
			return true;
		}
	}

	return false;
}

bool FRuntimeGameplaySettingsPropertyEntryCustomization::SetPropertyEntryHandleValue(
	TSharedRef<IPropertyHandle> InPropertyEntryHandle,
	const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry) const
{
	TArray<void*> RawData;
	InPropertyEntryHandle->AccessRawData(RawData);
	if (RawData.Num() != 1 || !RawData[0])
	{
		return false;
	}

	InPropertyEntryHandle->NotifyPreChange();
	*static_cast<FRuntimeGameplaySettingsPropertyEntry*>(RawData[0]) = PropertyEntry;
	InPropertyEntryHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
	InPropertyEntryHandle->NotifyFinishedChangingProperties();
	return true;
}

int32 FRuntimeGameplaySettingsPropertyEntryCustomization::FindCurrentPropertyIndex() const
{
	const FRuntimeGameplaySettingsClassEntry* ClassEntry = GetOwnerClassEntry();
	const FRuntimeGameplaySettingsPropertyEntry* PropertyEntry = GetPropertyEntry();
	if (!ClassEntry || !PropertyEntry)
	{
		return INDEX_NONE;
	}

	for (int32 PropertyIndex = 0; PropertyIndex < ClassEntry->Properties.Num(); ++PropertyIndex)
	{
		if (&ClassEntry->Properties[PropertyIndex] == PropertyEntry)
		{
			return PropertyIndex;
		}
	}

	for (int32 PropertyIndex = 0; PropertyIndex < ClassEntry->Properties.Num(); ++PropertyIndex)
	{
		const FRuntimeGameplaySettingsPropertyEntry& Candidate = ClassEntry->Properties[PropertyIndex];
		const FString CandidatePropertyPath =
			FRuntimeGameplaySettingsPropertyAccess::BuildPropertyPathString(
				Candidate.PropertyPath,
				Candidate.PropertyName);
		const FString CurrentPropertyPath =
			FRuntimeGameplaySettingsPropertyAccess::BuildPropertyPathString(
				PropertyEntry->PropertyPath,
				PropertyEntry->PropertyName);
		if (CandidatePropertyPath == CurrentPropertyPath
			&& Candidate.ComponentName == PropertyEntry->ComponentName
			&& Candidate.ValueType == PropertyEntry->ValueType)
		{
			return PropertyIndex;
		}
	}

	return INDEX_NONE;
}

FRuntimeGameplaySettingsPropertyEntry* FRuntimeGameplaySettingsPropertyEntryCustomization::GetMutablePropertyEntry() const
{
	if (!PropertyEntryHandle.IsValid())
	{
		return nullptr;
	}

	TArray<void*> RawData;
	PropertyEntryHandle->AccessRawData(RawData);
	return RawData.Num() == 1
		? static_cast<FRuntimeGameplaySettingsPropertyEntry*>(RawData[0])
		: nullptr;
}

const FRuntimeGameplaySettingsPropertyEntry* FRuntimeGameplaySettingsPropertyEntryCustomization::GetPropertyEntry() const
{
	return GetMutablePropertyEntry();
}

FRuntimeGameplaySettingsClassEntry* FRuntimeGameplaySettingsPropertyEntryCustomization::GetMutableOwnerClassEntry() const
{
	TSharedPtr<IPropertyHandle> ParentHandle =
		PropertyEntryHandle.IsValid() ? PropertyEntryHandle->GetParentHandle() : nullptr;
	while (ParentHandle.IsValid())
	{
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(ParentHandle->GetProperty()))
		{
			if (StructProperty->Struct == FRuntimeGameplaySettingsClassEntry::StaticStruct())
			{
				TArray<void*> RawData;
				ParentHandle->AccessRawData(RawData);
				return RawData.Num() == 1
					? static_cast<FRuntimeGameplaySettingsClassEntry*>(RawData[0])
					: nullptr;
			}
		}

		ParentHandle = ParentHandle->GetParentHandle();
	}

	return nullptr;
}

const FRuntimeGameplaySettingsClassEntry* FRuntimeGameplaySettingsPropertyEntryCustomization::GetOwnerClassEntry() const
{
	return GetMutableOwnerClassEntry();
}

#undef LOCTEXT_NAMESPACE
