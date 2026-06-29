using UnrealBuildTool;

public class RuntimeGameplaySettings : ModuleRules
{
	public RuntimeGameplaySettings(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"AssetRegistry",
			"DeveloperSettings",
			"Engine",
			"InputCore",
			"Slate",
			"SlateCore",
			"UMG"
		});
	}
}
