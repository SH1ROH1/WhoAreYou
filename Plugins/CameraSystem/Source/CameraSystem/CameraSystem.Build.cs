using UnrealBuildTool;
using System.IO;

public class CameraSystem : ModuleRules
{
    public CameraSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        // 1. Указываем компилятору, где искать публичные и приватные файлы плагина
        PublicIncludePaths.AddRange(new string[] {
            Path.Combine(ModuleDirectory, "Public")
        });

        PrivateIncludePaths.AddRange(new string[] {
            Path.Combine(ModuleDirectory, "Private")
        });

        // 2. Базовые модули движка, которые необходимы для работы ваших классов.
        // Если ваши классы используют EnhancedInput, Niagara или UI, добавьте их сюда!
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}
