using UnrealBuildTool;

public class RuntimeGameplaySettingsEditor : ModuleRules
{
	public RuntimeGameplaySettingsEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"RuntimeGameplaySettings"
			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"GraphEditor",
				"PropertyEditor",
				"Settings",
				"Slate",
				"SlateCore",
				"UnrealEd"
			});
	}
}
