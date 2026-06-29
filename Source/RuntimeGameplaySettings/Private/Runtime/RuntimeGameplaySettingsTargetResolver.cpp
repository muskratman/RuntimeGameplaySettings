#include "Runtime/RuntimeGameplaySettingsTargetResolver.h"

#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/HUD.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

TArray<UObject*> FRuntimeGameplaySettingsTargetResolver::ResolveTargets(
	APlayerController* PlayerController,
	UClass* TargetClass)
{
	TArray<UObject*> Targets;
	if (!PlayerController || !TargetClass)
	{
		return Targets;
	}

	const auto AddIfMatches = [&Targets, TargetClass](UObject* Candidate)
	{
		if (Candidate && Candidate->IsA(TargetClass))
		{
			Targets.AddUnique(Candidate);
		}
	};

	AddIfMatches(PlayerController);
	AddIfMatches(PlayerController->GetPawn());
	AddIfMatches(PlayerController->GetHUD());

	UWorld* World = PlayerController->GetWorld();
	if (World)
	{
		AddIfMatches(World->GetGameState());
		AddIfMatches(World->GetAuthGameMode());
	}

	return Targets;
}
