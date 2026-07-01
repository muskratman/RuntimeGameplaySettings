#include "Runtime/RuntimeGameplaySettingsPropertyAccess.h"

#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "UObject/EnumProperty.h"
#include "UObject/TextProperty.h"
#include "UObject/UnrealType.h"

namespace
{
bool TryGetObjectPropertyValue(UObject* Object, FName PropertyName, UObject*& OutValue)
{
	OutValue = nullptr;
	if (!Object)
	{
		return false;
	}

	const FObjectPropertyBase* ObjectProperty = FindFProperty<FObjectPropertyBase>(Object->GetClass(), PropertyName);
	if (!ObjectProperty)
	{
		return false;
	}

	OutValue = ObjectProperty->GetObjectPropertyValue_InContainer(Object);
	return OutValue != nullptr;
}

bool TryGetIntPropertyValue(UObject* Object, FName PropertyName, int32& OutValue)
{
	OutValue = INDEX_NONE;
	if (!Object)
	{
		return false;
	}

	const FIntProperty* IntProperty = FindFProperty<FIntProperty>(Object->GetClass(), PropertyName);
	if (!IntProperty)
	{
		return false;
	}

	OutValue = IntProperty->GetPropertyValue_InContainer(Object);
	return true;
}

bool TryGetFloatPropertyValue(UObject* Object, FName PropertyName, float& OutValue)
{
	OutValue = 0.0f;
	if (!Object)
	{
		return false;
	}

	const FFloatProperty* FloatProperty = FindFProperty<FFloatProperty>(Object->GetClass(), PropertyName);
	if (!FloatProperty)
	{
		return false;
	}

	OutValue = FloatProperty->GetPropertyValue_InContainer(Object);
	return true;
}

bool IsObjectAClassNamed(const UObject* Object, FName ExpectedClassName)
{
	for (const UClass* Class = Object ? Object->GetClass() : nullptr; Class; Class = Class->GetSuperClass())
	{
		if (Class->GetFName() == ExpectedClassName)
		{
			return true;
		}
	}

	return false;
}

bool TryNotifyChaosVehicleWheelChanged(UObject* PropertyOwner, const FProperty* ChangedProperty)
{
	if (!PropertyOwner
		|| !ChangedProperty
		|| !IsObjectAClassNamed(PropertyOwner, TEXT("ChaosVehicleWheel")))
	{
		return false;
	}

	UObject* VehicleComponent = nullptr;
	int32 WheelIndex = INDEX_NONE;
	if (!TryGetObjectPropertyValue(PropertyOwner, TEXT("VehicleComponent"), VehicleComponent)
		|| !TryGetIntPropertyValue(PropertyOwner, TEXT("WheelIndex"), WheelIndex)
		|| WheelIndex == INDEX_NONE)
	{
		return false;
	}

	if (ChangedProperty->GetFName() == TEXT("WheelRadius"))
	{
		float WheelRadius = 0.0f;
		UFunction* SetWheelRadiusFunction = VehicleComponent->FindFunction(TEXT("SetWheelRadius"));
		if (!SetWheelRadiusFunction
			|| !TryGetFloatPropertyValue(PropertyOwner, TEXT("WheelRadius"), WheelRadius))
		{
			return false;
		}

		struct FRuntimeGameplaySettingsSetWheelRadiusParams
		{
			int32 WheelIndex = INDEX_NONE;
			float Radius = 0.0f;
		};

		FRuntimeGameplaySettingsSetWheelRadiusParams Params;
		Params.WheelIndex = WheelIndex;
		Params.Radius = WheelRadius;
		VehicleComponent->ProcessEvent(SetWheelRadiusFunction, &Params);
		return true;
	}

	return false;
}
}

ERuntimeGameplaySettingsValueType FRuntimeGameplaySettingsPropertyAccess::GetSupportedValueType(
	const FProperty* Property,
	FString* OutEnumPath)
{
	if (!Property)
	{
		return ERuntimeGameplaySettingsValueType::Unsupported;
	}

	if (const FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
	{
		(void)BoolProperty;
		return ERuntimeGameplaySettingsValueType::Bool;
	}

	if (const FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property))
	{
		(void)FloatProperty;
		return ERuntimeGameplaySettingsValueType::Float;
	}

	if (const FIntProperty* IntProperty = CastField<FIntProperty>(Property))
	{
		(void)IntProperty;
		return ERuntimeGameplaySettingsValueType::Int;
	}

	if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
	{
		if (OutEnumPath && EnumProperty->GetEnum())
		{
			*OutEnumPath = EnumProperty->GetEnum()->GetPathName();
		}
		return ERuntimeGameplaySettingsValueType::Enum;
	}

	if (const FByteProperty* ByteProperty = CastField<FByteProperty>(Property))
	{
		if (ByteProperty->Enum)
		{
			if (OutEnumPath)
			{
				*OutEnumPath = ByteProperty->Enum->GetPathName();
			}
			return ERuntimeGameplaySettingsValueType::Enum;
		}
	}

	if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
	{
		if (StructProperty->Struct == TBaseStructure<FVector>::Get())
		{
			return ERuntimeGameplaySettingsValueType::Vector;
		}
		if (StructProperty->Struct == TBaseStructure<FRotator>::Get())
		{
			return ERuntimeGameplaySettingsValueType::Rotator;
		}
	}

	if (const FStrProperty* StringProperty = CastField<FStrProperty>(Property))
	{
		(void)StringProperty;
		return ERuntimeGameplaySettingsValueType::String;
	}

	if (const FNameProperty* NameProperty = CastField<FNameProperty>(Property))
	{
		(void)NameProperty;
		return ERuntimeGameplaySettingsValueType::Name;
	}

	return ERuntimeGameplaySettingsValueType::Unsupported;
}

bool FRuntimeGameplaySettingsPropertyAccess::IsSupportedEditableProperty(const FProperty* Property)
{
	if (!Property)
	{
		return false;
	}

	if (!Property->HasAnyPropertyFlags(CPF_Edit)
		|| Property->HasAnyPropertyFlags(CPF_Transient | CPF_Deprecated))
	{
		return false;
	}

	return GetSupportedValueType(Property) != ERuntimeGameplaySettingsValueType::Unsupported;
}

FRuntimeGameplaySettingsDiscoveredProperty FRuntimeGameplaySettingsPropertyAccess::BuildDiscoveredProperty(
	const FProperty* Property,
	FName ComponentName,
	UClass* ComponentClass)
{
	FRuntimeGameplaySettingsDiscoveredProperty DiscoveredProperty;
	if (!Property)
	{
		return DiscoveredProperty;
	}

	FString EnumPath;
	const bool bIsComponentProperty = !ComponentName.IsNone();
	const FText PropertyDisplayName = Property->GetDisplayNameText();

	DiscoveredProperty.bIsComponentProperty = bIsComponentProperty;
	DiscoveredProperty.ComponentName = ComponentName;
	DiscoveredProperty.ComponentClassName = ComponentClass ? ComponentClass->GetFName() : NAME_None;
	DiscoveredProperty.ComponentClassPath = ComponentClass ? ComponentClass->GetPathName() : FString();
	DiscoveredProperty.PropertyName = Property->GetFName();
	DiscoveredProperty.PropertyPath.Add(DiscoveredProperty.PropertyName);
	DiscoveredProperty.OwnerClassName = Property->GetOwnerClass()
		? Property->GetOwnerClass()->GetFName()
		: NAME_None;
	DiscoveredProperty.DisplayName = bIsComponentProperty
		? FText::Format(INVTEXT("{0}.{1}"), FText::FromName(ComponentName), PropertyDisplayName)
		: PropertyDisplayName;
	DiscoveredProperty.Category = FName(*Property->GetMetaData(TEXT("Category")));
	DiscoveredProperty.ValueType = GetSupportedValueType(Property, &EnumPath);
	DiscoveredProperty.EnumPath = EnumPath;
	return DiscoveredProperty;
}

FRuntimeGameplaySettingsPropertyEntry FRuntimeGameplaySettingsPropertyAccess::BuildPropertyEntry(
	const FRuntimeGameplaySettingsDiscoveredProperty& DiscoveredProperty)
{
	FRuntimeGameplaySettingsPropertyEntry PropertyEntry;
	PropertyEntry.bHasComponentSelection = true;
	PropertyEntry.bIsComponentProperty = DiscoveredProperty.bIsComponentProperty;
	PropertyEntry.ComponentName = DiscoveredProperty.ComponentName;
	PropertyEntry.ComponentClassName = DiscoveredProperty.ComponentClassName;
	PropertyEntry.ComponentClassPath = DiscoveredProperty.ComponentClassPath;
	PropertyEntry.PropertyName = DiscoveredProperty.PropertyName;
	PropertyEntry.PropertyPath = DiscoveredProperty.PropertyPath;
	PropertyEntry.OwnerClassName = DiscoveredProperty.OwnerClassName;
	PropertyEntry.DisplayName = DiscoveredProperty.DisplayName;
	PropertyEntry.Category = DiscoveredProperty.Category;
	PropertyEntry.ValueType = DiscoveredProperty.ValueType;
	PropertyEntry.EnumPath = DiscoveredProperty.EnumPath;
	return PropertyEntry;
}

FProperty* FRuntimeGameplaySettingsPropertyAccess::FindRuntimeProperty(
	UObject* TargetObject,
	const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry)
{
	FProperty* Property = nullptr;
	void* ValuePtr = nullptr;
	ResolveRuntimeProperty(TargetObject, PropertyEntry, Property, ValuePtr);
	return Property;
}

bool FRuntimeGameplaySettingsPropertyAccess::ReadValue(
	UObject* TargetObject,
	const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry,
	FRuntimeGameplaySettingsValue& OutValue)
{
	FProperty* Property = nullptr;
	void* ValuePtr = nullptr;
	if (!ResolveRuntimeProperty(TargetObject, PropertyEntry, Property, ValuePtr)
		|| !Property
		|| !ValuePtr)
	{
		return false;
	}

	OutValue = FRuntimeGameplaySettingsValue();
	OutValue.ValueType = PropertyEntry.ValueType;

	switch (PropertyEntry.ValueType)
	{
	case ERuntimeGameplaySettingsValueType::Bool:
		OutValue.BoolValue = CastFieldChecked<FBoolProperty>(Property)->GetPropertyValue(ValuePtr);
		return true;
	case ERuntimeGameplaySettingsValueType::Float:
		OutValue.FloatValue = CastFieldChecked<FFloatProperty>(Property)->GetPropertyValue(ValuePtr);
		return true;
	case ERuntimeGameplaySettingsValueType::Int:
		OutValue.IntValue = CastFieldChecked<FIntProperty>(Property)->GetPropertyValue(ValuePtr);
		return true;
	case ERuntimeGameplaySettingsValueType::Enum:
		if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
		{
			OutValue.IntValue = static_cast<int32>(EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(ValuePtr));
			return true;
		}
		if (const FByteProperty* ByteProperty = CastField<FByteProperty>(Property))
		{
			OutValue.IntValue = static_cast<int32>(ByteProperty->GetPropertyValue(ValuePtr));
			return true;
		}
		return false;
	case ERuntimeGameplaySettingsValueType::Vector:
		OutValue.VectorValue = *static_cast<FVector*>(ValuePtr);
		return true;
	case ERuntimeGameplaySettingsValueType::Rotator:
		OutValue.RotatorValue = *static_cast<FRotator*>(ValuePtr);
		return true;
	case ERuntimeGameplaySettingsValueType::String:
		OutValue.StringValue = CastFieldChecked<FStrProperty>(Property)->GetPropertyValue(ValuePtr);
		return true;
	case ERuntimeGameplaySettingsValueType::Name:
		OutValue.NameValue = CastFieldChecked<FNameProperty>(Property)->GetPropertyValue(ValuePtr);
		return true;
	default:
		return false;
	}
}

bool FRuntimeGameplaySettingsPropertyAccess::WriteValue(
	UObject* TargetObject,
	const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry,
	const FRuntimeGameplaySettingsValue& Value)
{
	FProperty* Property = nullptr;
	void* ValuePtr = nullptr;
	UObject* PropertyOwner = nullptr;
	if (!ResolveRuntimeProperty(TargetObject, PropertyEntry, Property, ValuePtr, nullptr, &PropertyOwner)
		|| !Property
		|| !ValuePtr
		|| Value.ValueType != PropertyEntry.ValueType)
	{
		return false;
	}

	bool bDidWriteValue = false;
	switch (PropertyEntry.ValueType)
	{
	case ERuntimeGameplaySettingsValueType::Bool:
		CastFieldChecked<FBoolProperty>(Property)->SetPropertyValue(ValuePtr, Value.BoolValue);
		bDidWriteValue = true;
		break;
	case ERuntimeGameplaySettingsValueType::Float:
		CastFieldChecked<FFloatProperty>(Property)->SetPropertyValue(ValuePtr, Value.FloatValue);
		bDidWriteValue = true;
		break;
	case ERuntimeGameplaySettingsValueType::Int:
		CastFieldChecked<FIntProperty>(Property)->SetPropertyValue(ValuePtr, Value.IntValue);
		bDidWriteValue = true;
		break;
	case ERuntimeGameplaySettingsValueType::Enum:
		if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
		{
			EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(ValuePtr, static_cast<int64>(Value.IntValue));
			bDidWriteValue = true;
			break;
		}
		if (const FByteProperty* ByteProperty = CastField<FByteProperty>(Property))
		{
			ByteProperty->SetPropertyValue(ValuePtr, static_cast<uint8>(Value.IntValue));
			bDidWriteValue = true;
			break;
		}
		return false;
	case ERuntimeGameplaySettingsValueType::Vector:
		*static_cast<FVector*>(ValuePtr) = Value.VectorValue;
		bDidWriteValue = true;
		break;
	case ERuntimeGameplaySettingsValueType::Rotator:
		*static_cast<FRotator*>(ValuePtr) = Value.RotatorValue;
		bDidWriteValue = true;
		break;
	case ERuntimeGameplaySettingsValueType::String:
		CastFieldChecked<FStrProperty>(Property)->SetPropertyValue(ValuePtr, Value.StringValue);
		bDidWriteValue = true;
		break;
	case ERuntimeGameplaySettingsValueType::Name:
		CastFieldChecked<FNameProperty>(Property)->SetPropertyValue(ValuePtr, Value.NameValue);
		bDidWriteValue = true;
		break;
	default:
		return false;
	}

	if (bDidWriteValue)
	{
		NotifyRuntimePropertyChanged(PropertyOwner, Property);
	}

	return bDidWriteValue;
}

TArray<FName> FRuntimeGameplaySettingsPropertyAccess::GetEffectivePropertyPath(
	const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry)
{
	if (PropertyEntry.PropertyPath.Num() > 0)
	{
		return PropertyEntry.PropertyPath;
	}

	TArray<FName> PropertyPath;
	if (!PropertyEntry.PropertyName.IsNone())
	{
		PropertyPath.Add(PropertyEntry.PropertyName);
	}
	return PropertyPath;
}

FString FRuntimeGameplaySettingsPropertyAccess::BuildPropertyPathString(
	const TArray<FName>& PropertyPath,
	FName FallbackPropertyName)
{
	if (PropertyPath.Num() == 0)
	{
		return FallbackPropertyName.IsNone()
			? FString()
			: FallbackPropertyName.ToString();
	}

	TArray<FString> PathSegments;
	PathSegments.Reserve(PropertyPath.Num());
	for (const FName PathSegment : PropertyPath)
	{
		if (!PathSegment.IsNone())
		{
			PathSegments.Add(PathSegment.ToString());
		}
	}

	return FString::Join(PathSegments, TEXT("."));
}

TArray<FString> FRuntimeGameplaySettingsPropertyAccess::BuildEnumOptions(
	const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry)
{
	TArray<FString> Options;
	const UEnum* Enum = ResolveEnum(PropertyEntry.EnumPath);
	if (!Enum)
	{
		return Options;
	}

	const int32 NumEnums = Enum->NumEnums();
	for (int32 EnumIndex = 0; EnumIndex < NumEnums; ++EnumIndex)
	{
		const FString OptionName = Enum->GetNameStringByIndex(EnumIndex);
		if (Enum->HasMetaData(TEXT("Hidden"), EnumIndex) || OptionName.EndsWith(TEXT("_MAX")))
		{
			continue;
		}

		const FText DisplayText = Enum->GetDisplayNameTextByIndex(EnumIndex);
		Options.Add(DisplayText.IsEmpty() ? OptionName : DisplayText.ToString());
	}

	return Options;
}

bool FRuntimeGameplaySettingsPropertyAccess::TryResolveEnumValueFromDisplayName(
	const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry,
	const FString& DisplayName,
	int32& OutValue)
{
	const UEnum* Enum = ResolveEnum(PropertyEntry.EnumPath);
	if (!Enum)
	{
		return false;
	}

	const int32 NumEnums = Enum->NumEnums();
	for (int32 EnumIndex = 0; EnumIndex < NumEnums; ++EnumIndex)
	{
		const FString OptionName = Enum->GetNameStringByIndex(EnumIndex);
		const FText DisplayText = Enum->GetDisplayNameTextByIndex(EnumIndex);
		const FString ResolvedDisplayName = DisplayText.IsEmpty() ? OptionName : DisplayText.ToString();
		if (ResolvedDisplayName.Equals(DisplayName, ESearchCase::CaseSensitive))
		{
			OutValue = static_cast<int32>(Enum->GetValueByIndex(EnumIndex));
			return true;
		}
	}

	return false;
}

FString FRuntimeGameplaySettingsPropertyAccess::GetEnumDisplayName(
	const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry,
	int32 Value)
{
	const UEnum* Enum = ResolveEnum(PropertyEntry.EnumPath);
	if (!Enum)
	{
		return FString::FromInt(Value);
	}

	const int32 EnumIndex = Enum->GetIndexByValue(Value);
	if (EnumIndex == INDEX_NONE)
	{
		return FString::FromInt(Value);
	}

	const FText DisplayText = Enum->GetDisplayNameTextByIndex(EnumIndex);
	return DisplayText.IsEmpty() ? Enum->GetNameStringByIndex(EnumIndex) : DisplayText.ToString();
}

const UEnum* FRuntimeGameplaySettingsPropertyAccess::ResolveEnum(const FString& EnumPath)
{
	const FString SanitizedEnumPath = EnumPath.TrimStartAndEnd();
	if (SanitizedEnumPath.IsEmpty())
	{
		return nullptr;
	}

	if (const UEnum* ExistingEnum = FindObject<UEnum>(nullptr, *SanitizedEnumPath))
	{
		return ExistingEnum;
	}

	return LoadObject<UEnum>(nullptr, *SanitizedEnumPath);
}

UObject* FRuntimeGameplaySettingsPropertyAccess::ResolvePropertyOwner(
	UObject* TargetObject,
	const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry)
{
	if (!TargetObject)
	{
		return nullptr;
	}

	if (PropertyEntry.ComponentName.IsNone())
	{
		return TargetObject;
	}

	AActor* Actor = Cast<AActor>(TargetObject);
	if (!Actor)
	{
		return nullptr;
	}

	UClass* ExpectedComponentClass = nullptr;
	if (!PropertyEntry.ComponentClassPath.IsEmpty())
	{
		ExpectedComponentClass =
			FSoftClassPath(PropertyEntry.ComponentClassPath).TryLoadClass<UActorComponent>();
	}

	const FName ExpectedComponentName = NormalizeComponentName(PropertyEntry.ComponentName);
	TArray<UActorComponent*> Components;
	Actor->GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		if (!Component)
		{
			continue;
		}

		const bool bNameMatches =
			NormalizeComponentName(Component->GetFName()) == ExpectedComponentName;
		const bool bClassMatches =
			!ExpectedComponentClass || Component->IsA(ExpectedComponentClass);
		if (bNameMatches && bClassMatches)
		{
			return Component;
		}
	}

	return nullptr;
}

bool FRuntimeGameplaySettingsPropertyAccess::ResolveRuntimeProperty(
	UObject* TargetObject,
	const FRuntimeGameplaySettingsPropertyEntry& PropertyEntry,
	FProperty*& OutProperty,
	void*& OutValuePtr,
	FString* OutRuntimeEnumPath,
	UObject** OutPropertyOwner)
{
	OutProperty = nullptr;
	OutValuePtr = nullptr;
	if (OutRuntimeEnumPath)
	{
		OutRuntimeEnumPath->Reset();
	}

	UObject* PropertyOwner = ResolvePropertyOwner(TargetObject, PropertyEntry);
	if (!PropertyOwner)
	{
		return false;
	}
	if (OutPropertyOwner)
	{
		*OutPropertyOwner = PropertyOwner;
	}

	const TArray<FName> PropertyPath = GetEffectivePropertyPath(PropertyEntry);
	if (PropertyPath.Num() == 0)
	{
		return false;
	}

	UStruct* CurrentStruct = PropertyOwner->GetClass();
	void* CurrentContainerPtr = PropertyOwner;
	for (int32 PathIndex = 0; PathIndex < PropertyPath.Num(); ++PathIndex)
	{
		if (!CurrentStruct || !CurrentContainerPtr || PropertyPath[PathIndex].IsNone())
		{
			return false;
		}

		FProperty* CurrentProperty = FindFProperty<FProperty>(CurrentStruct, PropertyPath[PathIndex]);
		if (!CurrentProperty)
		{
			return false;
		}

		void* CurrentValuePtr = CurrentProperty->ContainerPtrToValuePtr<void>(CurrentContainerPtr);
		const bool bIsLeafProperty = PathIndex == PropertyPath.Num() - 1;
		if (bIsLeafProperty)
		{
			FString RuntimeEnumPath;
			const ERuntimeGameplaySettingsValueType RuntimeType =
				GetSupportedValueType(CurrentProperty, &RuntimeEnumPath);
			if (RuntimeType != PropertyEntry.ValueType)
			{
				return false;
			}

			if (RuntimeType == ERuntimeGameplaySettingsValueType::Enum
				&& !PropertyEntry.EnumPath.IsEmpty()
				&& !RuntimeEnumPath.Equals(PropertyEntry.EnumPath, ESearchCase::IgnoreCase))
			{
				return false;
			}

			OutProperty = CurrentProperty;
			OutValuePtr = CurrentValuePtr;
			if (OutRuntimeEnumPath)
			{
				*OutRuntimeEnumPath = RuntimeEnumPath;
			}
			return true;
		}

		const FStructProperty* StructProperty = CastField<FStructProperty>(CurrentProperty);
		if (!StructProperty || !StructProperty->Struct)
		{
			return false;
		}

		CurrentStruct = StructProperty->Struct;
		CurrentContainerPtr = CurrentValuePtr;
	}

	return false;
}

void FRuntimeGameplaySettingsPropertyAccess::NotifyRuntimePropertyChanged(
	UObject* PropertyOwner,
	const FProperty* ChangedProperty)
{
	if (!PropertyOwner)
	{
		return;
	}

	if (TryNotifyChaosVehicleWheelChanged(PropertyOwner, ChangedProperty))
	{
		return;
	}

	if (USceneComponent* SceneComponent = Cast<USceneComponent>(PropertyOwner))
	{
		SceneComponent->UpdateComponentToWorld();
	}

	if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(PropertyOwner))
	{
		PrimitiveComponent->UpdateBounds();
		PrimitiveComponent->MarkRenderStateDirty();
	}

	if (UActorComponent* ActorComponent = Cast<UActorComponent>(PropertyOwner))
	{
		if (ActorComponent->IsRegistered())
		{
			ActorComponent->RecreatePhysicsState();
		}
		return;
	}

	if (AActor* Actor = Cast<AActor>(PropertyOwner))
	{
		if (USceneComponent* RootComponent = Actor->GetRootComponent())
		{
			RootComponent->UpdateComponentToWorld();
		}
	}
}

FName FRuntimeGameplaySettingsPropertyAccess::NormalizeComponentName(FName ComponentName)
{
	FString NameString = ComponentName.ToString();
	NameString.RemoveFromEnd(UActorComponent::ComponentTemplateNameSuffix);
	NameString.RemoveFromEnd(TEXT("_GEN_VARIABLE"));
	return FName(*NameString);
}
