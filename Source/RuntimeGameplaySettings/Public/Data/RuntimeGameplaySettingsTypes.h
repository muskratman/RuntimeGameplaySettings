#pragma once

#include "CoreMinimal.h"
#include "RuntimeGameplaySettingsTypes.generated.h"

UENUM(BlueprintType)
enum class ERuntimeGameplaySettingsValueType : uint8
{
	Unsupported,
	Bool,
	Float,
	Int,
	Enum,
	Vector,
	Rotator,
	String,
	Name
};

USTRUCT(BlueprintType)
struct RUNTIMEGAMEPLAYSETTINGS_API FRuntimeGameplaySettingsValue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	ERuntimeGameplaySettingsValueType ValueType = ERuntimeGameplaySettingsValueType::Unsupported;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	bool BoolValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	float FloatValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	int32 IntValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	FVector VectorValue = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	FRotator RotatorValue = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	FString StringValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	FName NameValue;
};

USTRUCT(BlueprintType)
struct RUNTIMEGAMEPLAYSETTINGS_API FRuntimeGameplaySettingsDiscoveredProperty
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	bool bIsComponentProperty = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	FName ComponentName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	FName ComponentClassName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	FString ComponentClassPath;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	FName PropertyName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	TArray<FName> PropertyPath;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	FName OwnerClassName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	FText DisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	FName Category;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	ERuntimeGameplaySettingsValueType ValueType = ERuntimeGameplaySettingsValueType::Unsupported;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	FString EnumPath;
};

USTRUCT(BlueprintType)
struct RUNTIMEGAMEPLAYSETTINGS_API FRuntimeGameplaySettingsPropertyEntry
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	bool bHasComponentSelection = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	bool bIsComponentProperty = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runtime Gameplay Settings")
	FName ComponentName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	FName ComponentClassName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	FString ComponentClassPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runtime Gameplay Settings")
	FName PropertyName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	TArray<FName> PropertyPath;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	FName OwnerClassName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	FText DisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	FName Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runtime Gameplay Settings")
	FString RuntimeCategory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	ERuntimeGameplaySettingsValueType ValueType = ERuntimeGameplaySettingsValueType::Unsupported;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings")
	FString EnumPath;
};

USTRUCT(BlueprintType)
struct RUNTIMEGAMEPLAYSETTINGS_API FRuntimeGameplaySettingsClassEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runtime Gameplay Settings")
	TSoftClassPtr<UObject> TargetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runtime Gameplay Settings", meta=(TitleProperty="DisplayName"))
	TArray<FRuntimeGameplaySettingsDiscoveredProperty> AvailableProperties;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runtime Gameplay Settings", meta=(DisplayName="SelectedProperties", TitleProperty="DisplayName"))
	TArray<FRuntimeGameplaySettingsPropertyEntry> Properties;
};

USTRUCT(BlueprintType)
struct RUNTIMEGAMEPLAYSETTINGS_API FRuntimeGameplaySettingsPropertySnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	bool bIsComponentProperty = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	FName ComponentName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	FString ComponentClassPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	FName PropertyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	TArray<FName> PropertyPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	ERuntimeGameplaySettingsValueType ValueType = ERuntimeGameplaySettingsValueType::Unsupported;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	FString EnumPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	FRuntimeGameplaySettingsValue Value;
};

USTRUCT(BlueprintType)
struct RUNTIMEGAMEPLAYSETTINGS_API FRuntimeGameplaySettingsObjectSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	FSoftClassPath TargetClassPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	TArray<FRuntimeGameplaySettingsPropertySnapshot> Properties;
};

USTRUCT(BlueprintType)
struct RUNTIMEGAMEPLAYSETTINGS_API FRuntimeGameplaySettingsSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	FSoftObjectPath ProfilePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	TArray<FRuntimeGameplaySettingsObjectSnapshot> Objects;
};

USTRUCT(BlueprintType)
struct RUNTIMEGAMEPLAYSETTINGS_API FRuntimeGameplaySettingsSlotDescriptor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	FGuid SlotId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	FDateTime CreatedAtUtc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Runtime Gameplay Settings")
	FDateTime UpdatedAtUtc;
};
