// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ue5FirstTest : ModuleRules
{
	public ue5FirstTest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay","PhysicsCore", "NavigationSystem", "Niagara" });
	}
}
