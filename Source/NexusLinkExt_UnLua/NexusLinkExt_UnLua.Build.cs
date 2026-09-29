// Copyright byteyang. All Rights Reserved.

using UnrealBuildTool;

/// <summary>
/// UnLua 运行时 Capability（eval/dofile/gc/hotreload 与 get/set_runtime_lua_*）与 FNexusLuaUtils。
/// WITH_UNLUA / UNLUA_VERSION_MAJOR 只在本模块定义，经 PublicDefinitions 传给依赖方。
/// </summary>
public class NexusLinkExt_UnLua : ModuleRules
{
	public NexusLinkExt_UnLua(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(new[] { "Core", "NexusLink", "Json", "JsonUtilities" });
		PrivateDependencyModuleNames.AddRange(new[] { "CoreUObject", "Engine" });

		string ProjectRoot = NexusLinkOptionalPlugins.FindProjectRoot(ModuleDirectory);
		var SearchDirs = NexusLinkOptionalPlugins.CollectPluginSearchDirs(ProjectRoot, this);
		var UnLua = NexusLinkOptionalPlugins.Opt(
			new[] { "UnLua.uplugin" }, "WITH_UNLUA", "WITH_UNLUA",
			pub: new[] { "Lua" }, rt: new[] { "UnLua" }, noDisable: true);
		bool bHasUnLua = NexusLinkOptionalPlugins.Apply(
			this, Target, UnLua, SearchDirs, ProjectRoot, bAllowEditorModules: false);
		NexusLinkOptionalPlugins.ApplyUnLuaVersionDefines(this, bHasUnLua, SearchDirs);
	}
}
