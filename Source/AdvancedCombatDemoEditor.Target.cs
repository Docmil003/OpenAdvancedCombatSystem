// Copyright © 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class AdvancedCombatDemoEditorTarget : TargetRules
{
	public AdvancedCombatDemoEditorTarget(TargetInfo Target) : base(Target)
	{
        bOverrideBuildEnvironment = true;

        Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;

		ExtraModuleNames.AddRange( new string[] { "AdvancedCombatDemo" } );
	}
}
