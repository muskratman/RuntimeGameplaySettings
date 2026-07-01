#include "Runtime/RuntimeGameplaySettingsTargetResolver.h"

#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/HUD.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "UObject/UObjectHash.h"

namespace
{
bool IsRuntimeTargetCandidate(UObject* Candidate)
{
	return IsValid(Candidate)
		&& !Candidate->IsTemplate()
		&& !Candidate->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject);
}

void AddIfMatches(TArray<UObject*>& Targets, UObject* Candidate, UClass* TargetClass)
{
	if (IsRuntimeTargetCandidate(Candidate) && Candidate->IsA(TargetClass))
	{
		Targets.AddUnique(Candidate);
	}
}

void AddMatchingTargetTree(TArray<UObject*>& Targets, UObject* RootObject, UClass* TargetClass)
{
	if (!IsRuntimeTargetCandidate(RootObject) || !TargetClass)
	{
		return;
	}

	AddIfMatches(Targets, RootObject, TargetClass);

	ForEachObjectWithOuter(
		RootObject,
		[&Targets, TargetClass](UObject* ChildObject)
		{
			AddIfMatches(Targets, ChildObject, TargetClass);
		},
		true,
		RF_ClassDefaultObject | RF_ArchetypeObject,
		EInternalObjectFlags::Garbage);
}
}

TArray<UObject*> FRuntimeGameplaySettingsTargetResolver::ResolveTargets(
	APlayerController* PlayerController,
	UClass* TargetClass)
{
	TArray<UObject*> Targets;
	if (!PlayerController || !TargetClass)
	{
		return Targets;
	}

	AddMatchingTargetTree(Targets, PlayerController, TargetClass);
	AddMatchingTargetTree(Targets, PlayerController->GetPawn(), TargetClass);
	AddMatchingTargetTree(Targets, PlayerController->GetHUD(), TargetClass);

	UWorld* World = PlayerController->GetWorld();
	if (World)
	{
		AddMatchingTargetTree(Targets, World->GetGameState(), TargetClass);
		AddMatchingTargetTree(Targets, World->GetAuthGameMode(), TargetClass);
	}

	return Targets;
}
