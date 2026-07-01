#include "RuntimeGameplaySettingsClassEntryCustomization.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "PropertyHandle.h"
#include "Runtime/RuntimeGameplaySettingsPropertyAccess.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FRuntimeGameplaySettingsClassEntryCustomization"

TSharedRef<IPropertyTypeCustomization> FRuntimeGameplaySettingsClassEntryCustomization::MakeInstance()
{
	return MakeShared<FRuntimeGameplaySettingsClassEntryCustomization>();
}

void FRuntimeGameplaySettingsClassEntryCustomization::CustomizeHeader(
	TSharedRef<IPropertyHandle> StructPropertyHandle,
	FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	ClassEntryHandle = StructPropertyHandle;
	PropertyUtilities = CustomizationUtils.GetPropertyUtilities();

	HeaderRow
		.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(250.0f)
		[
			SNew(STextBlock)
			.Text(this, &FRuntimeGameplaySettingsClassEntryCustomization::GetHeaderSummaryText)
			.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
		];
}

void FRuntimeGameplaySettingsClassEntryCustomization::CustomizeChildren(
	TSharedRef<IPropertyHandle> StructPropertyHandle,
	IDetailChildrenBuilder& ChildBuilder,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	ClassEntryHandle = StructPropertyHandle;
	PropertyUtilities = CustomizationUtils.GetPropertyUtilities();

	const TSharedPtr<IPropertyHandle> TargetClassHandle = StructPropertyHandle->GetChildHandle(
		GET_MEMBER_NAME_CHECKED(FRuntimeGameplaySettingsClassEntry, TargetClass));
	const TSharedPtr<IPropertyHandle> PropertiesHandle = StructPropertyHandle->GetChildHandle(
		GET_MEMBER_NAME_CHECKED(FRuntimeGameplaySettingsClassEntry, Properties));

	if (TargetClassHandle.IsValid())
	{
		ChildBuilder.AddProperty(TargetClassHandle.ToSharedRef());
	}

	if (PropertiesHandle.IsValid())
	{
		ChildBuilder.AddProperty(PropertiesHandle.ToSharedRef());
	}

	ChildBuilder.AddCustomRow(LOCTEXT("AvailablePropertiesFilter", "Available Properties Component PropertyPath ValueType"))
		.WholeRowContent()
		[
			BuildAvailablePropertiesWidget()
		];
}

TSharedRef<SWidget> FRuntimeGameplaySettingsClassEntryCustomization::BuildAvailablePropertiesWidget()
{
	TSharedRef<SWidget> BodyWidget = SNew(SBox)
		.MaxDesiredHeight(560.0f)
		[
			SNew(SScrollBox)
			.ConsumeMouseWheel(EConsumeMouseWheel::Always)
			.ScrollBarAlwaysVisible(true)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 4.0f, 0.0f, 4.0f)
				[
					SNew(SSearchBox)
					.InitialText(FText::FromString(AvailableSearchString))
					.HintText(LOCTEXT("SearchAvailablePropertiesHint", "Search AvailableProperties"))
					.OnTextChanged(this, &FRuntimeGameplaySettingsClassEntryCustomization::HandleAvailableSearchTextChanged)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 4.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.0f, 0.0f, 6.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(this, &FRuntimeGameplaySettingsClassEntryCustomization::GetAvailableSortModeText)
						.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.0f, 0.0f, 4.0f, 0.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("SortAvailableByComponent", "Component"))
						.OnClicked(this, &FRuntimeGameplaySettingsClassEntryCustomization::HandleSetAvailableSortMode, 0)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.0f, 0.0f, 4.0f, 0.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("SortAvailableByPropertyName", "PropertyPath"))
						.OnClicked(this, &FRuntimeGameplaySettingsClassEntryCustomization::HandleSetAvailableSortMode, 1)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(LOCTEXT("SortAvailableByValueType", "ValueType"))
						.OnClicked(this, &FRuntimeGameplaySettingsClassEntryCustomization::HandleSetAvailableSortMode, 2)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(AvailablePropertiesBox, SVerticalBox)
				]
			]
		];

	TSharedRef<SWidget> Widget = SNew(SExpandableArea)
		.InitiallyCollapsed(true)
		.AreaTitle(this, &FRuntimeGameplaySettingsClassEntryCustomization::GetAvailableHeaderText)
		.BodyContent()
		[
			BodyWidget
		];

	RebuildAvailablePropertiesList();
	return Widget;
}

TSharedRef<SWidget> FRuntimeGameplaySettingsClassEntryCustomization::BuildAvailablePropertyRow(
	const FRuntimeGameplaySettingsDiscoveredProperty& DiscoveredProperty,
	bool bIsHeader) const
{
	const FText ComponentText = bIsHeader
		? LOCTEXT("AvailableComponentHeader", "Component")
		: (DiscoveredProperty.ComponentName.IsNone()
			? LOCTEXT("ActorPropertyComponent", "Actor")
			: FText::FromName(DiscoveredProperty.ComponentName));
	const FText PropertyText = bIsHeader
		? LOCTEXT("AvailablePropertyNameHeader", "PropertyPath")
		: FText::FromString(FRuntimeGameplaySettingsPropertyAccess::BuildPropertyPathString(
			DiscoveredProperty.PropertyPath,
			DiscoveredProperty.PropertyName));
	const FText ValueTypeText = bIsHeader
		? LOCTEXT("AvailableValueTypeHeader", "ValueType")
		: GetValueTypeText(DiscoveredProperty.ValueType);

	return SNew(SBorder)
		.Padding(FMargin(4.0f, 2.0f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.MinWidth(260.0f)
			[
				SNew(STextBlock)
				.Text(ComponentText)
				.Font(bIsHeader
					? IPropertyTypeCustomizationUtils::GetBoldFont()
					: IPropertyTypeCustomizationUtils::GetRegularFont())
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.MinWidth(320.0f)
			[
				SNew(STextBlock)
				.Text(PropertyText)
				.Font(bIsHeader
					? IPropertyTypeCustomizationUtils::GetBoldFont()
					: IPropertyTypeCustomizationUtils::GetRegularFont())
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.MinWidth(120.0f)
			[
				SNew(STextBlock)
				.Text(ValueTypeText)
				.Font(bIsHeader
					? IPropertyTypeCustomizationUtils::GetBoldFont()
					: IPropertyTypeCustomizationUtils::GetRegularFont())
			]
		];
}

void FRuntimeGameplaySettingsClassEntryCustomization::RebuildAvailablePropertiesList()
{
	if (!AvailablePropertiesBox.IsValid())
	{
		return;
	}

	AvailablePropertiesBox->ClearChildren();

	const FRuntimeGameplaySettingsClassEntry* ClassEntry = GetClassEntry();
	if (!ClassEntry)
	{
		AvailablePropertiesBox->AddSlot()
		.AutoHeight()
		.Padding(4.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NoAvailableClassEntry", "No class entry data."))
			.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
		];
		return;
	}

	if (ClassEntry->AvailableProperties.Num() == 0)
	{
		AvailablePropertiesBox->AddSlot()
		.AutoHeight()
		.Padding(4.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NoAvailablePropertiesFound", "No AvailableProperties. Press Refresh All Properties if the list is empty."))
			.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
		];
		return;
	}

	TArray<int32> FilteredIndices;
	const FString NormalizedSearch = AvailableSearchString.TrimStartAndEnd().ToLower();
	for (int32 AvailableIndex = 0; AvailableIndex < ClassEntry->AvailableProperties.Num(); ++AvailableIndex)
	{
		const FRuntimeGameplaySettingsDiscoveredProperty& DiscoveredProperty =
			ClassEntry->AvailableProperties[AvailableIndex];
		const FString Label = GetDiscoveredPropertyLabel(DiscoveredProperty).ToString();
		const FString ValueType = GetValueTypeText(DiscoveredProperty.ValueType).ToString();
		const FString PropertyPath =
			FRuntimeGameplaySettingsPropertyAccess::BuildPropertyPathString(
				DiscoveredProperty.PropertyPath,
				DiscoveredProperty.PropertyName);
		const FString SearchHaystack = FString::Printf(
			TEXT("%s %s %s %s %s %s %s"),
			*Label,
			*PropertyPath,
			*DiscoveredProperty.ComponentName.ToString(),
			*DiscoveredProperty.ComponentClassName.ToString(),
			*DiscoveredProperty.PropertyName.ToString(),
			*DiscoveredProperty.OwnerClassName.ToString(),
			*ValueType);
		if (!NormalizedSearch.IsEmpty() && !SearchHaystack.ToLower().Contains(NormalizedSearch))
		{
			continue;
		}

		FilteredIndices.Add(AvailableIndex);
	}

	FilteredIndices.Sort([this, ClassEntry](int32 LeftIndex, int32 RightIndex)
	{
		const FRuntimeGameplaySettingsDiscoveredProperty& Left =
			ClassEntry->AvailableProperties[LeftIndex];
		const FRuntimeGameplaySettingsDiscoveredProperty& Right =
			ClassEntry->AvailableProperties[RightIndex];

		const auto ComponentLabel = [](const FRuntimeGameplaySettingsDiscoveredProperty& Property)
		{
			return Property.ComponentName.IsNone()
				? FString(TEXT("Actor"))
				: Property.ComponentName.ToString();
		};

		FString LeftPrimary;
		FString RightPrimary;
		switch (AvailableSortMode)
		{
		case 1:
			LeftPrimary = FRuntimeGameplaySettingsPropertyAccess::BuildPropertyPathString(
				Left.PropertyPath,
				Left.PropertyName);
			RightPrimary = FRuntimeGameplaySettingsPropertyAccess::BuildPropertyPathString(
				Right.PropertyPath,
				Right.PropertyName);
			break;
		case 2:
			LeftPrimary = GetValueTypeText(Left.ValueType).ToString();
			RightPrimary = GetValueTypeText(Right.ValueType).ToString();
			break;
		default:
			LeftPrimary = ComponentLabel(Left);
			RightPrimary = ComponentLabel(Right);
			break;
		}

		const int32 PrimaryCompare = LeftPrimary.Compare(RightPrimary, ESearchCase::IgnoreCase);
		if (PrimaryCompare != 0)
		{
			return PrimaryCompare < 0;
		}

		const int32 ComponentCompare = ComponentLabel(Left).Compare(ComponentLabel(Right), ESearchCase::IgnoreCase);
		if (ComponentCompare != 0)
		{
			return ComponentCompare < 0;
		}

		const FString LeftPropertyPath =
			FRuntimeGameplaySettingsPropertyAccess::BuildPropertyPathString(
				Left.PropertyPath,
				Left.PropertyName);
		const FString RightPropertyPath =
			FRuntimeGameplaySettingsPropertyAccess::BuildPropertyPathString(
				Right.PropertyPath,
				Right.PropertyName);
		return LeftPropertyPath.Compare(RightPropertyPath, ESearchCase::IgnoreCase) < 0;
	});

	AvailablePropertiesBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 2.0f)
	[
		BuildAvailablePropertyRow(FRuntimeGameplaySettingsDiscoveredProperty(), true)
	];

	if (FilteredIndices.Num() == 0)
	{
		AvailablePropertiesBox->AddSlot()
		.AutoHeight()
		.Padding(4.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NoMatchingAvailableProperties", "No matching AvailableProperties."))
			.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
		];
		return;
	}

	for (int32 AvailableIndex : FilteredIndices)
	{
		AvailablePropertiesBox->AddSlot()
		.AutoHeight()
		[
			BuildAvailablePropertyRow(ClassEntry->AvailableProperties[AvailableIndex], false)
		];
	}
}

void FRuntimeGameplaySettingsClassEntryCustomization::HandleAvailableSearchTextChanged(const FText& InSearchText)
{
	AvailableSearchString = InSearchText.ToString();
	RebuildAvailablePropertiesList();
}

FReply FRuntimeGameplaySettingsClassEntryCustomization::HandleSetAvailableSortMode(int32 SortMode)
{
	AvailableSortMode = FMath::Clamp(SortMode, 0, 2);
	RebuildAvailablePropertiesList();
	return FReply::Handled();
}

FText FRuntimeGameplaySettingsClassEntryCustomization::GetHeaderSummaryText() const
{
	const FRuntimeGameplaySettingsClassEntry* ClassEntry = GetClassEntry();
	if (!ClassEntry)
	{
		return LOCTEXT("NoClassEntrySummary", "No data");
	}

	const FString ClassName = ClassEntry->TargetClass.IsNull()
		? FString(TEXT("No TargetClass"))
		: ClassEntry->TargetClass.GetAssetName();
	return FText::Format(
		LOCTEXT("ClassEntrySummary", "{0} - {1} selected"),
		FText::FromString(ClassName),
		FText::AsNumber(ClassEntry->Properties.Num()));
}

FText FRuntimeGameplaySettingsClassEntryCustomization::GetAvailableHeaderText() const
{
	const FRuntimeGameplaySettingsClassEntry* ClassEntry = GetClassEntry();
	const int32 NumProperties = ClassEntry ? ClassEntry->AvailableProperties.Num() : 0;
	return FText::Format(
		LOCTEXT("AvailablePropertiesHeader", "AvailableProperties ({0})"),
		FText::AsNumber(NumProperties));
}

FText FRuntimeGameplaySettingsClassEntryCustomization::GetAvailableSortModeText() const
{
	switch (AvailableSortMode)
	{
	case 1:
		return LOCTEXT("AvailableSortModePropertyName", "Sort: PropertyPath");
	case 2:
		return LOCTEXT("AvailableSortModeValueType", "Sort: ValueType");
	default:
		return LOCTEXT("AvailableSortModeComponent", "Sort: Component");
	}
}

FText FRuntimeGameplaySettingsClassEntryCustomization::GetDiscoveredPropertyLabel(
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

FText FRuntimeGameplaySettingsClassEntryCustomization::GetValueTypeText(
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

FRuntimeGameplaySettingsClassEntry* FRuntimeGameplaySettingsClassEntryCustomization::GetMutableClassEntry() const
{
	if (!ClassEntryHandle.IsValid())
	{
		return nullptr;
	}

	TArray<void*> RawData;
	ClassEntryHandle->AccessRawData(RawData);
	return RawData.Num() == 1
		? static_cast<FRuntimeGameplaySettingsClassEntry*>(RawData[0])
		: nullptr;
}

const FRuntimeGameplaySettingsClassEntry* FRuntimeGameplaySettingsClassEntryCustomization::GetClassEntry() const
{
	return GetMutableClassEntry();
}

#undef LOCTEXT_NAMESPACE
