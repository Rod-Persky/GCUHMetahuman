// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class SG_Com : ModuleRules
{
    public SG_Com(ReadOnlyTargetRules Target) : base(Target)
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
                // ... add other public dependencies that you statically link with here ...
            }
            );
            
        
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                // ... add private dependencies that you statically link with here ...    
            }
            );
        
        
        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
            );

        if (Target.Platform == UnrealTargetPlatform.Win64) {
            string dll_path = Path.Combine(PluginDirectory, "Binaries/Win64/SG_Com.dll");
            string lib_path = Path.Combine(PluginDirectory, "Binaries/Win64/SG_Com.lib");
            PublicDelayLoadDLLs.Add(dll_path);
            PublicAdditionalLibraries.Add(lib_path);

            RuntimeDependencies.Add("$(TargetOutputDir)/SG_Com.dll", dll_path);
        }
        else if (Target.Platform == UnrealTargetPlatform.Android) {
            AdditionalPropertiesForReceipt.Add("AndroidPlugin", PluginDirectory + "/Source/SG_Com/SG_Com_APL.xml");
            PublicAdditionalLibraries.Add(PluginDirectory + "/Binaries/Android/armeabi-v7a/libSG_Com.so");
            PublicAdditionalLibraries.Add(PluginDirectory + "/Binaries/Android/arm64-v8a/libSG_Com.so");
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac) {
            string lib_path = Path.Combine(PluginDirectory, "Binaries/Mac/libSG_Com.a");
            PublicAdditionalLibraries.Add(lib_path);
        }
        else if (Target.Platform == UnrealTargetPlatform.IOS) {
            string lib_path = Path.Combine(PluginDirectory, "Binaries/IOS/libSG_Com.a");
            PublicAdditionalLibraries.Add(lib_path);
        }
    }
}
