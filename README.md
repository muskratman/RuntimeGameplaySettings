# Runtime Gameplay Settings

Runtime Gameplay Settings is an Unreal Engine plugin for exposing selected gameplay `UPROPERTY` values to a local runtime tuning widget. It is intended as a debug/runtime-dev tool for PIE and local development, letting designers and programmers adjust selected Pawn, PlayerController, GameMode, HUD, and GameState settings without adding custom UI or input code to each gameplay class.

## Overview

The plugin is built around a `URuntimeGameplaySettingsProfile` data asset. A profile contains class entries, and each entry points to a Blueprint class that should be inspected. The editor customization discovers editable properties on that class, inherited C++ or Blueprint parents, and its Blueprint component templates. Selected properties are then rendered in a generic runtime widget.

At runtime, the plugin resolves matching objects from the local player context:

- the local `PlayerController`
- the controlled `Pawn`
- the controller `HUD`
- the current `GameState`
- the authoritative `GameMode`, when available

The runtime widget is generated from the profile. Each selected property becomes a value row, and each `Widget category` value becomes a tab. The `Common` category always exists and receives properties with an empty category.

The plugin also includes a local-player subsystem that can bind a configurable hotkey, `U` by default, to open and close the widget without modifying the project's `PlayerController` class.

## Current Feature Set

- Project Settings section: `Project Settings -> GameplaySettings -> Runtime Gameplay Settings`.
- Manual settings registration, with `UDeveloperSettings` auto-registration disabled.
- Data asset based profile: `URuntimeGameplaySettingsProfile`.
- Blueprint target class picker per profile entry.
- Component-first and property-second pickers for selected properties.
- Discovery of editable properties from target classes, inherited parents, Blueprint component templates, and supported nested editable structs.
- Runtime-generated widget categories and property rows.
- Default `Common` category.
- Save, Save As, Load, Delete, Reset, and Reset All runtime controls.
- Optional pause while the runtime widget is open.
- Configurable toggle key, input consumption, and execute-when-paused behavior.
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
- `Widget Class` to a custom `URuntimeGameplaySettingsWidget` subclass, or leave the default C++ widget.
- `Enable Toggle Hotkey` to `true` if the widget should open from a key press.
- `Toggle Key` to the desired input key. The default is `U`.
- `Consume Input` and `Execute When Paused` as needed.
- `Pause Game While Open` if the game should pause while the panel is visible.

### 3. Run PIE or local play

Start PIE or a local game session. Press the configured toggle key to open the runtime widget.

The widget builds tabs from the selected properties in the configured profile. Editing values changes the widget working copy. Saving or closing the widget applies the current values to matching runtime targets.

### 4. Save and reload tuning snapshots

The widget can save snapshots into Unreal save-game slots:

- `Save` updates the current slot.
- `Save As` creates a named slot.
- `Load` restores a selected slot.
- `Delete` removes a selected slot.
- `Reset All` returns visible properties to their captured runtime defaults.

Saved snapshots are profile-aware and are applied back through the selected property paths.

## Notes and Limitations

- This plugin is designed for local debug and runtime-dev workflows, not for shipping user-facing settings menus.
- The toggle input is registered by the local-player subsystem using an internal `UInputComponent`; project `PlayerController` code does not need a manual binding.
- Runtime application depends on the selected target class matching one of the objects resolved from the local player context.
- Properties must be editable `UPROPERTY` fields and must use one of the supported runtime value types.
- Generated folders such as `Binaries`, `Intermediate`, `Saved`, and `DerivedDataCache` should not be versioned.
