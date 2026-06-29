#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Tickable.h"
#include "RuntimeGameplaySettingsLocalPlayerSubsystem.generated.h"

class APlayerController;
class UInputComponent;
class URuntimeGameplaySettingsProfile;
class URuntimeGameplaySettingsWidget;

UCLASS()
class RUNTIMEGAMEPLAYSETTINGS_API URuntimeGameplaySettingsLocalPlayerSubsystem
	: public ULocalPlayerSubsystem
	, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;

	void ToggleRuntimeGameplaySettingsWidget();
	void CloseRuntimeGameplaySettingsWidget();
	bool ApplyCurrentRuntimeGameplaySettings();

private:
	APlayerController* GetRuntimePlayerController() const;
	URuntimeGameplaySettingsProfile* LoadConfiguredProfile() const;
	bool AreRuntimeGameplaySettingsEnabled() const;
	void UpdateRuntimeBinding();
	void RegisterToggleInput(APlayerController* PlayerController);
	void UnregisterToggleInput();
	void ApplyCurrentSettingsForControllerIfNeeded(APlayerController* PlayerController);

	UPROPERTY(Transient)
	TObjectPtr<UInputComponent> ToggleInputComponent;

	UPROPERTY(Transient)
	TObjectPtr<URuntimeGameplaySettingsWidget> ActiveWidget;

	TWeakObjectPtr<APlayerController> BoundPlayerController;
	TWeakObjectPtr<APawn> LastAppliedPawn;
	bool bAppliedForCurrentController = false;
};
