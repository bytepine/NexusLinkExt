// Copyright byteyang. All Rights Reserved.

using UnrealBuildTool;

/// <summary>
/// UnLua 编辑器 Capability（get/manage_asset_lua_binding）。
/// WITH_UNLUA / UNLUA_VERSION_MAJOR 由 NexusLinkExt_UnLua 传入，这里只补链 UnLua，避免宏重定义。
/// </summary>
public class NexusLinkExtEditor_UnLua : ModuleRules
{
	public NexusLinkExtEditor_UnLua(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "NexusLink", "NexusLinkExt_UnLua", "Json", "JsonUtilities",
		});
		PrivateDependencyModuleNames.AddRange(new[]
		{
			"CoreUObject", "Engine", "UnrealEd",
		});

		string ProjectRoot = NexusLinkOptionalPlugins.FindProjectRoot(ModuleDirectory);
		var SearchDirs = NexusLinkOptionalPlugins.CollectPluginSearchDirs(ProjectRoot, this);
		var UnLua = NexusLinkOptionalPlugins.Opt(
			new[] { "UnLua.uplugin" }, "WITH_UNLUA", "WITH_UNLUA",
			pub: new[] { "Lua" }, rt: new[] { "UnLua" }, noDisable: true);
		NexusLinkOptionalPlugins.Apply(
			this, Target, UnLua, SearchDirs, ProjectRoot,
			bAllowEditorModules: false, bDefineMacro: false);
	}
}
