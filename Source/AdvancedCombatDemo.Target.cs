// Copyright © 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class AdvancedCombatDemoTarget : TargetRules
{
	public AdvancedCombatDemoTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		//DefaultBuildSettings = BuildSettingsVersion.V2;
        DefaultBuildSettings = BuildSettingsVersion.Latest;

        ExtraModuleNames.AddRange( new string[] { "AdvancedCombatDemo" } );
	}
}
