// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class WhoAreYou : ModuleRules
{
	public WhoAreYou(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore"
        });

                PrivateDependencyModuleNames.AddRange(new string[]
                {
            "UMG",
            "Slate",
            "SlateCore",
            "CameraSystem"
        });

    }
}
