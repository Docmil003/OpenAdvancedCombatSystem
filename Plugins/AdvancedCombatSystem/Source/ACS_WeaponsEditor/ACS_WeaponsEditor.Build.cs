// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

using UnrealBuildTool;

public class ACS_WeaponsEditor : ModuleRules
{
	public ACS_WeaponsEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "ACS_Core",
				"MessageLog",
				"PropertyEditor"

				// ... add other public dependencies that you statically link with here ...
			}
            );
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
                "PhysicsCore",
                "Engine",
				"Slate",
                "SlateCore",
				"UMG",
                "JsonUtilities"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
