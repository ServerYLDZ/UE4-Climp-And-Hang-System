// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OyunDeneme : ModuleRules
{
	public OyunDeneme(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
	}
}
