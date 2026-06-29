#include "Subsystems/RuntimeGameplaySettingsLocalPlayerSubsystem.h"

#include "Components/InputComponent.h"
#include "Data/RuntimeGameplaySettingsProfile.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Runtime/RuntimeGameplaySettingsSnapshotLibrary.h"
#include "Settings/RuntimeGameplaySettingsProjectSettings.h"
#include "Subsystems/RuntimeGameplaySettingsSubsystem.h"
#include "UI/RuntimeGameplaySettingsWidget.h"

void URuntimeGameplaySettingsLocalPlayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void URuntimeGameplaySettingsLocalPlayerSubsystem::Deinitialize()
{
	CloseRuntimeGameplaySettingsWidget();
	UnregisterToggleInput();
	Super::Deinitialize();
}

void URuntimeGameplaySettingsLocalPlayerSubsystem::Tick(float DeltaTime)
{
	(void)DeltaTime;
	UpdateRuntimeBinding();
}

TStatId URuntimeGameplaySettingsLocalPlayerSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URuntimeGameplaySettingsLocalPlayerSubsystem, STATGROUP_Tickables);
}

bool URuntimeGameplaySettingsLocalPlayerSubsystem::IsTickable() const
{
	return !IsTemplate();
}

void URuntimeGameplaySettingsLocalPlayerSubsystem::ToggleRuntimeGameplaySettingsWidget()
{
	if (ActiveWidget && ActiveWidget->IsInViewport())
	{
		ActiveWidget->CloseRuntimeGameplaySettingsWidget();
		return;
	}

	if (!AreRuntimeGameplaySettingsEnabled())
	{
		return;
	}

	APlayerController* PlayerController = GetRuntimePlayerController();
	URuntimeGameplaySettingsProfile* Profile = LoadConfiguredProfile();
	if (!PlayerController || !Profile)
	{
		return;
	}

	const URuntimeGameplaySettingsProjectSettings* Settings =
		GetDefault<URuntimeGameplaySettingsProjectSettings>();

	TSubclassOf<URuntimeGameplaySettingsWidget> WidgetClass = Settings->WidgetClass.LoadSynchronous();
	if (!WidgetClass)
	{
		WidgetClass = URuntimeGameplaySettingsWidget::StaticClass();
	}

	ActiveWidget = CreateWidget<URuntimeGameplaySettingsWidget>(PlayerController, WidgetClass.Get());
	if (!ActiveWidget)
	{
		return;
	}

	UGameInstance* GameInstance = PlayerController->GetGameInstance();
	URuntimeGameplaySettingsSubsystem* SettingsSubsystem =
		GameInstance ? GameInstance->GetSubsystem<URuntimeGameplaySettingsSubsystem>() : nullptr;

	ActiveWidget->OnRequestClose.BindUObject(
		this,
		&URuntimeGameplaySettingsLocalPlayerSubsystem::CloseRuntimeGameplaySettingsWidget);
	ActiveWidget->InitializeRuntimeGameplaySettings(PlayerController, Profile, SettingsSubsystem);
	ActiveWidget->AddToViewport(1000);

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(ActiveWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;
	if (Settings->bPauseGameWhileOpen)
	{
		PlayerController->SetPause(true);
	}
}

void URuntimeGameplaySettingsLocalPlayerSubsystem::CloseRuntimeGameplaySettingsWidget()
{
	if (ActiveWidget)
	{
		ActiveWidget->RemoveFromParent();
		ActiveWidget = nullptr;
	}

	if (APlayerController* PlayerController = GetRuntimePlayerController())
	{
		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);
		PlayerController->bShowMouseCursor = false;

		const URuntimeGameplaySettingsProjectSettings* Settings =
			GetDefault<URuntimeGameplaySettingsProjectSettings>();
		if (Settings->bPauseGameWhileOpen)
		{
			PlayerController->SetPause(false);
		}
	}
}

bool URuntimeGameplaySettingsLocalPlayerSubsystem::ApplyCurrentRuntimeGameplaySettings()
{
	if (!AreRuntimeGameplaySettingsEnabled())
	{
		return false;
	}

	APlayerController* PlayerController = GetRuntimePlayerController();
	URuntimeGameplaySettingsProfile* Profile = LoadConfiguredProfile();
	if (!PlayerController || !Profile)
	{
		return false;
	}

	UGameInstance* GameInstance = PlayerController->GetGameInstance();
	URuntimeGameplaySettingsSubsystem* SettingsSubsystem =
		GameInstance ? GameInstance->GetSubsystem<URuntimeGameplaySettingsSubsystem>() : nullptr;
	if (!SettingsSubsystem)
	{
		return false;
	}

	if (!SettingsSubsystem->HasBaselineSnapshot())
	{
		SettingsSubsystem->SetBaselineSnapshot(
			FRuntimeGameplaySettingsSnapshotLibrary::CaptureSnapshot(PlayerController, Profile));
	}

	FRuntimeGameplaySettingsSnapshot Snapshot;
	if (!SettingsSubsystem->TryLoadCurrentSnapshot(Snapshot))
	{
		return false;
	}

	FRuntimeGameplaySettingsSnapshotLibrary::ApplySnapshot(PlayerController, Profile, Snapshot);
	return true;
}

APlayerController* URuntimeGameplaySettingsLocalPlayerSubsystem::GetRuntimePlayerController() const
{
	const ULocalPlayer* RuntimeLocalPlayer = GetLocalPlayer();
	if (!RuntimeLocalPlayer)
	{
		return nullptr;
	}

	return RuntimeLocalPlayer->GetPlayerController(GetWorld());
}

URuntimeGameplaySettingsProfile* URuntimeGameplaySettingsLocalPlayerSubsystem::LoadConfiguredProfile() const
{
	const URuntimeGameplaySettingsProjectSettings* Settings =
		GetDefault<URuntimeGameplaySettingsProjectSettings>();
	return Settings ? Settings->DefaultProfile.LoadSynchronous() : nullptr;
}

bool URuntimeGameplaySettingsLocalPlayerSubsystem::AreRuntimeGameplaySettingsEnabled() const
{
	const URuntimeGameplaySettingsProjectSettings* Settings =
		GetDefault<URuntimeGameplaySettingsProjectSettings>();
	return Settings
		&& Settings->bEnableRuntimeGameplaySettings
		&& !Settings->DefaultProfile.IsNull();
}

void URuntimeGameplaySettingsLocalPlayerSubsystem::UpdateRuntimeBinding()
{
	if (!AreRuntimeGameplaySettingsEnabled())
	{
		CloseRuntimeGameplaySettingsWidget();
		UnregisterToggleInput();
		LastAppliedPawn = nullptr;
		bAppliedForCurrentController = false;
		return;
	}

	APlayerController* PlayerController = GetRuntimePlayerController();
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		UnregisterToggleInput();
		return;
	}

	if (BoundPlayerController.Get() != PlayerController)
	{
		UnregisterToggleInput();
		RegisterToggleInput(PlayerController);
		BoundPlayerController = PlayerController;
		LastAppliedPawn = nullptr;
		bAppliedForCurrentController = false;
	}

	ApplyCurrentSettingsForControllerIfNeeded(PlayerController);
}

void URuntimeGameplaySettingsLocalPlayerSubsystem::RegisterToggleInput(APlayerController* PlayerController)
{
	const URuntimeGameplaySettingsProjectSettings* Settings =
		GetDefault<URuntimeGameplaySettingsProjectSettings>();
	if (!PlayerController || !Settings || !Settings->bEnableToggleHotkey || !Settings->ToggleKey.IsValid())
	{
		return;
	}

	ToggleInputComponent = NewObject<UInputComponent>(PlayerController, TEXT("RuntimeGameplaySettingsInputComponent"));
	if (!ToggleInputComponent)
	{
		return;
	}

	ToggleInputComponent->RegisterComponent();
	FInputKeyBinding& Binding = ToggleInputComponent->BindKey(
		Settings->ToggleKey,
		IE_Pressed,
		this,
		&URuntimeGameplaySettingsLocalPlayerSubsystem::ToggleRuntimeGameplaySettingsWidget);
	Binding.bConsumeInput = Settings->bConsumeInput;
	Binding.bExecuteWhenPaused = Settings->bExecuteWhenPaused;
	PlayerController->PushInputComponent(ToggleInputComponent);
}

void URuntimeGameplaySettingsLocalPlayerSubsystem::UnregisterToggleInput()
{
	if (APlayerController* PlayerController = BoundPlayerController.Get())
	{
		if (ToggleInputComponent)
		{
			PlayerController->PopInputComponent(ToggleInputComponent);
		}
	}

	if (ToggleInputComponent)
	{
		ToggleInputComponent->DestroyComponent();
		ToggleInputComponent = nullptr;
	}

	BoundPlayerController = nullptr;
}

void URuntimeGameplaySettingsLocalPlayerSubsystem::ApplyCurrentSettingsForControllerIfNeeded(
	APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	APawn* CurrentPawn = PlayerController->GetPawn();
	if (!bAppliedForCurrentController || LastAppliedPawn.Get() != CurrentPawn)
	{
		ApplyCurrentRuntimeGameplaySettings();
		LastAppliedPawn = CurrentPawn;
		bAppliedForCurrentController = true;
	}
}
