#include "RuntimeGameplaySettingsProfileCustomization.h"

#include "Components/ActorComponent.h"
#include "Data/RuntimeGameplaySettingsProfile.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameFramework/Actor.h"
#include "Runtime/RuntimeGameplaySettingsPropertyAccess.h"
#include "ScopedTransaction.h"
#include "UObject/UnrealType.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FRuntimeGameplaySettingsProfileCustomization"

namespace
{
FName NormalizeComponentTemplateName(FName ComponentName)
{
	FString NameString = ComponentName.ToString();
	NameString.RemoveFromEnd(UActorComponent::ComponentTemplateNameSuffix);
	NameString.RemoveFromEnd(TEXT("_GEN_VARIABLE"));
	return FName(*NameString);
}

FString BuildDiscoveryKey(const FRuntimeGameplaySettingsDiscoveredProperty& DiscoveredProperty)
{
	return FString::Printf(
		TEXT("%s|%s"),
		*DiscoveredProperty.ComponentName.ToString(),
		*FRuntimeGameplaySettingsPropertyAccess::BuildPropertyPathString(
			DiscoveredProperty.PropertyPath,
			DiscoveredProperty.PropertyName));
}

FText GetPropertyDisplayName(const FProperty* Property)
{
	if (!Property)
	{
		return FText::GetEmpty();
	}

	const FText DisplayName = Property->GetDisplayNameText();
	return DisplayName.IsEmpty()
		? FText::FromName(Property->GetFName())
		: DisplayName;
}

FText BuildPropertyPathDisplayText(const TArray<FText>& DisplayPath)
{
	TArray<FString> DisplaySegments;
	DisplaySegments.Reserve(DisplayPath.Num());
	for (const FText& DisplaySegment : DisplayPath)
	{
		const FString DisplayString = DisplaySegment.ToString();
		if (!DisplayString.IsEmpty())
		{
			DisplaySegments.Add(DisplayString);
		}
	}

	return FText::FromString(FString::Join(DisplaySegments, TEXT(".")));
}

bool CanDiscoverNestedStructProperty(const FStructProperty* StructProperty, int32 CurrentDepth)
{
	constexpr int32 MaxNestedStructDepth = 4;
	return StructProperty
		&& StructProperty->Struct
		&& CurrentDepth < MaxNestedStructDepth
		&& StructProperty->HasAnyPropertyFlags(CPF_Edit)
		&& !StructProperty->HasAnyPropertyFlags(CPF_Transient | CPF_Deprecated)
		&& FRuntimeGameplaySettingsPropertyAccess::GetSupportedValueType(StructProperty)
			== ERuntimeGameplaySettingsValueType::Unsupported;
}

void AddDiscoveredProperty(
	TArray<FRuntimeGameplaySettingsDiscoveredProperty>& AvailableProperties,
	TSet<FString>& AddedKeys,
	const FProperty* Property,
	FName ComponentName,
	UClass* ComponentClass,
	const TArray<FName>& PropertyPath,
	const TArray<FText>& DisplayPath)
{
	if (!Property || !FRuntimeGameplaySettingsPropertyAccess::IsSupportedEditableProperty(Property))
	{
		return;
	}

	FRuntimeGameplaySettingsDiscoveredProperty DiscoveredProperty =
		FRuntimeGameplaySettingsPropertyAccess::BuildDiscoveredProperty(
			Property,
			ComponentName,
			ComponentClass);

	DiscoveredProperty.PropertyPath = PropertyPath;
	DiscoveredProperty.PropertyName = Property->GetFName();
	if (const UStruct* OwnerStruct = Property->GetOwnerStruct())
	{
		DiscoveredProperty.OwnerClassName = OwnerStruct->GetFName();
	}

	if (PropertyPath.Num() > 1)
	{
		DiscoveredProperty.Category = PropertyPath[0];
	}

	const FText PathDisplayText = BuildPropertyPathDisplayText(DisplayPath);
	DiscoveredProperty.DisplayName = ComponentName.IsNone()
		? PathDisplayText
		: FText::Format(
			INVTEXT("{0}.{1}"),
			FText::FromName(ComponentName),
			PathDisplayText);

	const FString DiscoveryKey = BuildDiscoveryKey(DiscoveredProperty);
	if (!AddedKeys.Contains(DiscoveryKey))
	{
		AddedKeys.Add(DiscoveryKey);
		AvailableProperties.Add(DiscoveredProperty);
	}
}

void AddSupportedPropertiesFromStruct(
	TArray<FRuntimeGameplaySettingsDiscoveredProperty>& AvailableProperties,
	TSet<FString>& AddedKeys,
	UStruct* PropertyOwnerStruct,
	FName ComponentName,
	UClass* ComponentClass,
	const TArray<FName>& ParentPropertyPath,
	const TArray<FText>& ParentDisplayPath,
	int32 CurrentDepth)
{
	if (!PropertyOwnerStruct)
	{
		return;
	}

	for (TFieldIterator<FProperty> PropertyIt(
		PropertyOwnerStruct,
		EFieldIteratorFlags::IncludeSuper);
		PropertyIt;
		++PropertyIt)
	{
		const FProperty* Property = *PropertyIt;
		TArray<FName> PropertyPath = ParentPropertyPath;
		PropertyPath.Add(Property->GetFName());

		TArray<FText> DisplayPath = ParentDisplayPath;
		DisplayPath.Add(GetPropertyDisplayName(Property));

		AddDiscoveredProperty(
			AvailableProperties,
			AddedKeys,
			Property,
			ComponentName,
			ComponentClass,
			PropertyPath,
			DisplayPath);

		const FStructProperty* StructProperty = CastField<FStructProperty>(Property);
		if (CanDiscoverNestedStructProperty(StructProperty, CurrentDepth))
		{
			AddSupportedPropertiesFromStruct(
				AvailableProperties,
				AddedKeys,
				StructProperty->Struct,
				ComponentName,
				ComponentClass,
				PropertyPath,
				DisplayPath,
				CurrentDepth + 1);
		}
	}
}

void AddSupportedPropertiesFromClass(
	TArray<FRuntimeGameplaySettingsDiscoveredProperty>& AvailableProperties,
	TSet<FString>& AddedKeys,
	UClass* PropertyOwnerClass,
	FName ComponentName = NAME_None,
	UClass* ComponentClass = nullptr)
{
	if (!PropertyOwnerClass)
	{
		return;
	}

	AddSupportedPropertiesFromStruct(
		AvailableProperties,
		AddedKeys,
		PropertyOwnerClass,
		ComponentName,
		ComponentClass,
		TArray<FName>(),
		TArray<FText>(),
		0);
}

void AddSupportedPropertiesFromComponentTemplate(
	TArray<FRuntimeGameplaySettingsDiscoveredProperty>& AvailableProperties,
	TSet<FString>& AddedKeys,
	UActorComponent* ComponentTemplate,
	FName ComponentName)
{
	if (!ComponentTemplate)
	{
		return;
	}

	const FName NormalizedComponentName = NormalizeComponentTemplateName(
		ComponentName.IsNone() ? ComponentTemplate->GetFName() : ComponentName);
	if (NormalizedComponentName.IsNone())
	{
		return;
	}

	AddSupportedPropertiesFromClass(
		AvailableProperties,
		AddedKeys,
		ComponentTemplate->GetClass(),
		NormalizedComponentName,
		ComponentTemplate->GetClass());
}

void AddBlueprintComponentProperties(
	TArray<FRuntimeGameplaySettingsDiscoveredProperty>& AvailableProperties,
	TSet<FString>& AddedKeys,
	UClass* TargetClass)
{
	UBlueprintGeneratedClass* ActualBlueprintClass = Cast<UBlueprintGeneratedClass>(TargetClass);
	if (!ActualBlueprintClass)
	{
		return;
	}

	UBlueprintGeneratedClass::ForEachGeneratedClassInHierarchy(
		TargetClass,
		[&AvailableProperties, &AddedKeys, ActualBlueprintClass](const UBlueprintGeneratedClass* BlueprintClass)
		{
			if (BlueprintClass->SimpleConstructionScript)
			{
				for (USCS_Node* Node : BlueprintClass->SimpleConstructionScript->GetAllNodes())
				{
					if (!Node)
					{
						continue;
					}

					UActorComponent* ComponentTemplate =
						Node->GetActualComponentTemplate(ActualBlueprintClass);
					if (!ComponentTemplate)
					{
						ComponentTemplate = Node->ComponentTemplate;
					}

					AddSupportedPropertiesFromComponentTemplate(
						AvailableProperties,
						AddedKeys,
						ComponentTemplate,
						Node->GetVariableName());
				}
			}

			for (UActorComponent* ComponentTemplate : BlueprintClass->ComponentTemplates)
			{
				AddSupportedPropertiesFromComponentTemplate(
					AvailableProperties,
					AddedKeys,
					ComponentTemplate,
					ComponentTemplate ? ComponentTemplate->GetFName() : NAME_None);
			}

			return true;
		});
}

void AddDefaultActorComponentProperties(
	TArray<FRuntimeGameplaySettingsDiscoveredProperty>& AvailableProperties,
	TSet<FString>& AddedKeys,
	UClass* TargetClass)
{
	AActor* DefaultActor = TargetClass ? Cast<AActor>(TargetClass->GetDefaultObject()) : nullptr;
	if (!DefaultActor)
	{
		return;
	}

	TArray<UActorComponent*> Components;
	DefaultActor->GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		AddSupportedPropertiesFromComponentTemplate(
			AvailableProperties,
			AddedKeys,
			Component,
			Component ? Component->GetFName() : NAME_None);
	}
}

TArray<FName> GetEffectiveSelectedPropertyPath(const FRuntimeGameplaySettingsPropertyEntry& SelectedProperty)
{
	if (SelectedProperty.PropertyPath.Num() > 0)
	{
		return SelectedProperty.PropertyPath;
	}

	TArray<FName> PropertyPath;
	if (!SelectedProperty.PropertyName.IsNone())
	{
		PropertyPath.Add(SelectedProperty.PropertyName);
	}
	return PropertyPath;
}

bool DoPropertyPathsMatch(
	const FRuntimeGameplaySettingsDiscoveredProperty& Candidate,
	const FRuntimeGameplaySettingsPropertyEntry& SelectedProperty)
{
	const TArray<FName> SelectedPropertyPath = GetEffectiveSelectedPropertyPath(SelectedProperty);
	if (Candidate.PropertyPath.Num() != SelectedPropertyPath.Num())
	{
		return false;
	}

	for (int32 PathIndex = 0; PathIndex < Candidate.PropertyPath.Num(); ++PathIndex)
	{
		if (Candidate.PropertyPath[PathIndex] != SelectedPropertyPath[PathIndex])
		{
			return false;
		}
	}

	return true;
}

const FRuntimeGameplaySettingsDiscoveredProperty* FindMatchingDiscoveredProperty(
	const TArray<FRuntimeGameplaySettingsDiscoveredProperty>& AvailableProperties,
	const FRuntimeGameplaySettingsPropertyEntry& SelectedProperty)
{
	const FRuntimeGameplaySettingsDiscoveredProperty* ExactMatch =
		AvailableProperties.FindByPredicate(
			[&SelectedProperty](const FRuntimeGameplaySettingsDiscoveredProperty& Candidate)
			{
				return DoPropertyPathsMatch(Candidate, SelectedProperty)
					&& Candidate.ComponentName == SelectedProperty.ComponentName;
			});
	if (ExactMatch)
	{
		return ExactMatch;
	}

	if (SelectedProperty.ComponentName.IsNone())
	{
		return AvailableProperties.FindByPredicate(
			[&SelectedProperty](const FRuntimeGameplaySettingsDiscoveredProperty& Candidate)
			{
				return DoPropertyPathsMatch(Candidate, SelectedProperty)
					&& Candidate.ComponentName.IsNone();
			});
	}

	return nullptr;
}
}

TSharedRef<IDetailCustomization> FRuntimeGameplaySettingsProfileCustomization::MakeInstance()
{
	return MakeShared<FRuntimeGameplaySettingsProfileCustomization>();
}

void FRuntimeGameplaySettingsProfileCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	CustomizedProfiles.Reset();

	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
	DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);
	for (const TWeakObjectPtr<UObject>& CustomizedObject : CustomizedObjects)
	{
		if (URuntimeGameplaySettingsProfile* Profile =
			Cast<URuntimeGameplaySettingsProfile>(CustomizedObject.Get()))
		{
			CustomizedProfiles.Add(Profile);
		}
	}

	IDetailCategoryBuilder& Category =
		DetailBuilder.EditCategory("Runtime Gameplay Settings");
	Category.AddCustomRow(LOCTEXT("RefreshPropertiesFilter", "Refresh Properties"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("RefreshPropertiesLabel", "Property Discovery"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		.MinDesiredWidth(180.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("RefreshPropertiesButton", "Refresh All Properties"))
			.ToolTipText(LOCTEXT(
				"RefreshPropertiesTooltip",
				"Scans each selected TargetClass and updates AvailableProperties plus metadata for already selected Properties."))
			.OnClicked(this, &FRuntimeGameplaySettingsProfileCustomization::HandleRefreshPropertiesClicked)
		];
}

FReply FRuntimeGameplaySettingsProfileCustomization::HandleRefreshPropertiesClicked()
{
	const FScopedTransaction Transaction(
		LOCTEXT("RefreshRuntimeGameplaySettingsProperties", "Refresh Runtime Gameplay Settings Properties"));

	for (const TWeakObjectPtr<URuntimeGameplaySettingsProfile>& ProfilePtr : CustomizedProfiles)
	{
		if (URuntimeGameplaySettingsProfile* Profile = ProfilePtr.Get())
		{
			RefreshProfileProperties(Profile);
		}
	}

	return FReply::Handled();
}

void FRuntimeGameplaySettingsProfileCustomization::RefreshProfileProperties(
	URuntimeGameplaySettingsProfile* Profile) const
{
	if (!Profile)
	{
		return;
	}

	Profile->Modify();

	for (FRuntimeGameplaySettingsClassEntry& ClassEntry : Profile->RuntimeGameplaySettingsEntries)
	{
		ClassEntry.AvailableProperties.Reset();
		TSet<FString> AddedPropertyKeys;

		UClass* TargetClass = ClassEntry.TargetClass.LoadSynchronous();
		if (!TargetClass)
		{
			continue;
		}

		AddSupportedPropertiesFromClass(
			ClassEntry.AvailableProperties,
			AddedPropertyKeys,
			TargetClass);
		AddBlueprintComponentProperties(
			ClassEntry.AvailableProperties,
			AddedPropertyKeys,
			TargetClass);
		AddDefaultActorComponentProperties(
			ClassEntry.AvailableProperties,
			AddedPropertyKeys,
			TargetClass);

		ClassEntry.AvailableProperties.Sort(
			[](const FRuntimeGameplaySettingsDiscoveredProperty& Left,
				const FRuntimeGameplaySettingsDiscoveredProperty& Right)
			{
				if (Left.ComponentName != Right.ComponentName)
				{
					if (Left.ComponentName.IsNone())
					{
						return true;
					}
					if (Right.ComponentName.IsNone())
					{
						return false;
					}

					return Left.ComponentName.LexicalLess(Right.ComponentName);
				}

				const int32 CategoryCompare =
					Left.Category.ToString().Compare(Right.Category.ToString());
				if (CategoryCompare != 0)
				{
					return CategoryCompare < 0;
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

		for (FRuntimeGameplaySettingsPropertyEntry& SelectedProperty : ClassEntry.Properties)
		{
			const FRuntimeGameplaySettingsDiscoveredProperty* DiscoveredProperty =
				FindMatchingDiscoveredProperty(ClassEntry.AvailableProperties, SelectedProperty);

			if (DiscoveredProperty)
			{
				const FString RuntimeCategory = SelectedProperty.RuntimeCategory;
				const FString SubCategory = SelectedProperty.SubCategory;
				SelectedProperty = FRuntimeGameplaySettingsPropertyAccess::BuildPropertyEntry(*DiscoveredProperty);
				SelectedProperty.RuntimeCategory = RuntimeCategory;
				SelectedProperty.SubCategory = SubCategory;
			}
		}
	}

	Profile->MarkPackageDirty();
}

#undef LOCTEXT_NAMESPACE
