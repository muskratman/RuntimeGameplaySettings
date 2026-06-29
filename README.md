# Runtime Gameplay Settings

Runtime Gameplay Settings is an Unreal Engine plugin that simplifies access to gameplay settings for game designers by exposing selected gameplay `UPROPERTY` values to a local runtime tuning widget. It is intended as a debug/runtime-dev tool for PIE and local development, letting designers and programmers adjust selected Pawn, PlayerController, GameMode, HUD, and GameState settings without adding custom UI or input code to each gameplay class. The plugin supports settings profiles and can save tuned values into multiple named slots, making it easy to compare, restore, and reuse different gameplay configurations.

## Overview

The plugin is built around a `URuntimeGameplaySettingsProfile` data asset. A profile contains class entries, and each entry points to a Blueprint class that should be inspected. The editor customization discovers editable properties on that class, inherited C++ or Blueprint parents, Blueprint component templates, and supported nested editable structs. Selected properties are then rendered in a generic runtime widget.

At runtime, the plugin resolves matching objects from the local player context:

- the local `PlayerController`
- the controlled `Pawn`
- the controller `HUD`
- the current `GameState`
- the authoritative `GameMode`, when available

The runtime widget is generated from the profile. Each selected property becomes a value row, each `Widget category` value becomes a tab, and each component/object group is rendered as a centered section title inside that tab. The `Common` category always exists and receives properties with an empty category.

The plugin also includes a local-player subsystem that can bind a configurable hotkey, `U` by default, to open and close the widget without modifying the project's `PlayerController` class.

## Current Feature Set

- Project Settings section: `Project Settings -> GameplaySettings -> Runtime Gameplay Settings`.
- Manual settings registration, with `UDeveloperSettings` auto-registration disabled.
- Data asset based profile: `URuntimeGameplaySettingsProfile`.
- Blueprint target class picker per profile entry.
- Component-first and property-second pickers for selected properties.
- Discovery of editable properties from target classes, inherited parents, Blueprint component templates, and supported nested editable structs.
- Runtime-generated widget categories, component/object groups, and property rows.
- Default `Common` category.
- UPROPERTY tooltip forwarding to runtime value rows.
- Save, Save As, Load, Delete, per-property Reset, and Reset All runtime controls.
- Baseline reset support: Reset and Reset All return automatic profile properties to the captured pre-runtime-tuning values from the start of the local session.
- Optional pause while the runtime widget is open.
- Configurable toggle key, input consumption, and execute-when-paused behavior.
- Configurable widget classes for the main widget, panels, category pages, component groups, category buttons, reset buttons, and value elements.
- C++ extension hooks for adding manual categories and parameters in code.
- Automatic re-application of saved settings when the local controller/pawn context changes, for example after respawn.

Supported runtime property types:

- `bool`
- `float`
- `int32`
- enum and byte enum properties
- `FVector`
- `FRotator`
- `FString`
- `FName`

## Installation

1. Copy the `RuntimeGameplaySettings` folder into your project's `Plugins` directory.
2. Make sure the plugin is enabled in the Unreal Editor Plugin Browser.
3. Regenerate project files if needed.
4. Build the project editor target.

The plugin contains both runtime and editor modules:

- `RuntimeGameplaySettings`
- `RuntimeGameplaySettingsEditor`

Do not commit generated Unreal build output. In particular, `Binaries` and `Intermediate` are temporary folders and should stay ignored.

## Basic Usage

### 1. Create a Runtime Gameplay Settings profile

Create a new data asset of class `RuntimeGameplaySettingsProfile`.

In `RuntimeGameplaySettingsEntries`, add one or more entries. For each entry:

1. Set `TargetClass` to the Blueprint class you want to tune.
2. Click the profile refresh button to rebuild the discovered property list after changing target classes.
3. Add entries to `SelectedProperties`.
4. For each selected property, choose a component first, then choose a property from that component.
5. Optionally set `Widget category`. Empty values are treated as `Common`.

You can inspect the discovered property table in the asset details panel. It is useful for checking the component name, property name, and detected value type.

### 2. Configure Project Settings

Open:

`Project Settings -> GameplaySettings -> Runtime Gameplay Settings`

Set:

- `Enable Runtime Gameplay Settings` to `true`.
- `Default Profile` to your `RuntimeGameplaySettingsProfile` asset.
- `Pause Game While Open` if the game should pause while the panel is visible.
- `Widget Class` to a custom `URuntimeGameplaySettingsWidget` subclass, or leave the default C++ widget.
- Optional UI widget classes if you want to replace the default panels or value elements.
- `Enable Toggle Hotkey` to `true` if the widget should open from a key press.
- `Toggle Key` to the desired input key. The default is `U`.
- `Consume Input` and `Execute When Paused` as needed.

### 3. Run PIE or local play

Start PIE or a local game session. Press the configured toggle key to open the runtime widget.

The widget builds tabs from the selected properties in the configured profile. Editing values changes the widget working copy. Saving or closing the widget applies the current values to matching runtime targets.

### 4. Save and reload tuning snapshots

The widget can save snapshots into Unreal save-game slots:

- `Save` updates the current slot.
- `Save As` creates a named slot.
- `Load` restores a selected slot.
- `Delete` removes a selected slot.
- Per-property `Reset` returns that row to its captured baseline value in the widget.
- `Reset All` immediately applies the captured baseline snapshot back to the matching runtime targets and rebuilds the widget.

Saved snapshots are profile-aware and are applied back through the selected property paths.

## Manual C++ Categories and Parameters

Automatic rows come from `URuntimeGameplaySettingsProfile`. Programmers can also add extra categories and parameters manually by deriving from `URuntimeGameplaySettingsWidget`.

Use this flow:

1. Create a C++ subclass of `URuntimeGameplaySettingsWidget`.
2. Override `BuildManualRows()` to add extra categories, groups, and parameters.
3. Set `Project Settings -> GameplaySettings -> Runtime Gameplay Settings -> Widget Class` to your subclass.
4. If manual values must affect runtime objects, override `ApplyRuntimeGameplaySettingsToTargets()`. Call `Super::ApplyRuntimeGameplaySettingsToTargets()` first if you still want the automatic DataAsset-driven rows to be applied.

Available protected helpers:

- `GetOrCreateManualCategoryContentBox(CategoryName)`
- `GetOrCreateManualParameterGroupContentBox(CategoryName, GroupDisplayName)`
- `AddManualParameterWidget(CategoryName, GroupDisplayName, Widget, TooltipText)`
- `AddManualBoolParameter(...)`
- `AddManualFloatParameter(...)`
- `AddManualIntParameter(...)`
- `AddManualEnumParameter(...)`
- `AddManualVectorParameter(...)`
- `AddManualRotatorParameter(...)`

The typed `AddManual...Parameter` helpers create the default Runtime Gameplay Settings element widgets, assign the display name, current value, default value, optional tooltip, and return the created widget pointer. Store returned pointers in your subclass if you need to read their values later.

Example:

```cpp
#include "UI/RuntimeGameplaySettingsWidget.h"
#include "UI/Elements/RuntimeGameplaySettingsCheckboxWidget.h"
#include "UI/Elements/RuntimeGameplaySettingsParameterWidget.h"

UCLASS()
class UMyRuntimeGameplaySettingsWidget : public URuntimeGameplaySettingsWidget
{
	GENERATED_BODY()

protected:
	virtual void BuildManualRows() override
	{
		Super::BuildManualRows();

		bDebugCameraWidget = AddManualBoolParameter(
			TEXT("Camera"),
			TEXT("Debug"),
			INVTEXT("Debug Camera"),
			bDebugCamera,
			false,
			INVTEXT("Enables an extra local camera debug mode."));

		CameraFovWidget = AddManualFloatParameter(
			TEXT("Camera"),
			TEXT("Debug"),
			INVTEXT("Debug FOV"),
			CameraFov,
			90.0f,
			30.0f,
			140.0f,
			0.5f);
	}

	virtual bool ApplyRuntimeGameplaySettingsToTargets() override
	{
		const bool bAppliedAutomaticRows = Super::ApplyRuntimeGameplaySettingsToTargets();

		if (bDebugCameraWidget)
		{
			bDebugCamera = bDebugCameraWidget->GetCheckboxValue();
		}

		if (CameraFovWidget)
		{
			CameraFov = CameraFovWidget->GetParameterValue();
		}

		return bAppliedAutomaticRows;
	}

private:
	UPROPERTY(Transient)
	TObjectPtr<URuntimeGameplaySettingsCheckboxWidget> bDebugCameraWidget;

	UPROPERTY(Transient)
	TObjectPtr<URuntimeGameplaySettingsParameterWidget> CameraFovWidget;

	bool bDebugCamera = false;
	float CameraFov = 90.0f;
};
```

Manual parameters are UI-level rows. They are not automatically included in profile snapshots because they do not have a `UPROPERTY` path in the configured `URuntimeGameplaySettingsProfile`. Persist or apply manual values explicitly in your subclass when needed.

## Customizing Widget Classes

The Project Settings section exposes soft class pickers for the main runtime widget and the default UI building blocks:

- main widget: `URuntimeGameplaySettingsWidget`
- save-slot panel
- buttons overlay
- category tabs
- category page
- component group
- category button
- bool, number, enum, vector, curve, curve-asset, float-array, and float-array-element widgets
- reset button widget

This makes it possible to keep the plugin portable across engine versions while still replacing the default visual structure with project-specific subclasses.

## Reset Behavior

The plugin captures a baseline snapshot before saved settings are applied for the current local session. Automatic profile rows use that snapshot as their reset default.

- Per-property `Reset` changes the row's widget value back to its baseline value.
- `Save`, `Save As`, or closing the widget applies the current widget values to runtime targets.
- `Reset All` applies the full baseline snapshot to runtime targets immediately and rebuilds the widget.

If no baseline entry is available for a row, the widget falls back to the value read from the runtime object when that row is built.

## Notes and Limitations

- This plugin is designed for local debug and runtime-dev workflows, not for shipping user-facing settings menus.
- The toggle input is registered by the local-player subsystem using an internal `UInputComponent`; project `PlayerController` code does not need a manual binding.
- Runtime application depends on the selected target class matching one of the objects resolved from the local player context.
- Properties must be editable `UPROPERTY` fields and must use one of the supported runtime value types.
- Manual C++ parameters must be applied and persisted by the custom widget subclass.
- Generated folders such as `Binaries`, `Intermediate`, `Saved`, and `DerivedDataCache` should not be versioned.
