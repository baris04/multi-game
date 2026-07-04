using UnrealBuildTool;

public class MultiGame : ModuleRules
{
	public MultiGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore",
			"AIModule",
			"GameplayTasks",
			"NavigationSystem"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});

		// Allow "Core/Foo.h" style includes from the module root (no Public/Private split).
		PublicIncludePaths.Add(ModuleDirectory);
	}
}
