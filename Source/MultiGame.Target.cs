using UnrealBuildTool;
using System.Collections.Generic;

public class MultiGameTarget : TargetRules
{
	public MultiGameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("MultiGame");
	}
}
