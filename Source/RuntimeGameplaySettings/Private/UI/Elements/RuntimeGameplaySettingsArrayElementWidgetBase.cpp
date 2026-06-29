#include "UI/Elements/RuntimeGameplaySettingsArrayElementWidgetBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"

void URuntimeGameplaySettingsArrayElementWidgetBase::SetElementIndex(int32 InElementIndex)
{
	ElementIndex = InElementIndex;
}

int32 URuntimeGameplaySettingsArrayElementWidgetBase::GetElementIndex() const
{
	return ElementIndex;
}

void URuntimeGameplaySettingsArrayElementWidgetBase::NativePreConstruct()
{
	EnsureDefaultWidgetTree();
	Super::NativePreConstruct();
}

void URuntimeGameplaySettingsArrayElementWidgetBase::NativeOnInitialized()
{
	EnsureDefaultWidgetTree();
	Super::NativeOnInitialized();

	if (DeleteButt)
	{
		DeleteButt->OnReleased.AddDynamic(this, &URuntimeGameplaySettingsArrayElementWidgetBase::HandleDeleteReleased);
	}
}

void URuntimeGameplaySettingsArrayElementWidgetBase::NotifyElementValueChanged()
{
	OnRuntimeGameplaySettingsArrayElementValueChanged.Broadcast(this);
}

void URuntimeGameplaySettingsArrayElementWidgetBase::HandleDeleteReleased()
{
	OnRuntimeGameplaySettingsArrayElementDeleteRequested.Broadcast(this);
}

void URuntimeGameplaySettingsArrayElementWidgetBase::BuildDefaultWidgetTree()
{
}

void URuntimeGameplaySettingsArrayElementWidgetBase::EnsureDefaultWidgetTree()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	BuildDefaultWidgetTree();
}
