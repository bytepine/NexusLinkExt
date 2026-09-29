// Copyright byteyang. All Rights Reserved.

using UnrealBuildTool;

/// <summary>
/// NexusLinkExtEditor（Editor）：可选插件的编辑器 Capability。当前是 UnLua 蓝图绑定
/// （get/manage_asset_lua_binding）。WITH_UNLUA / UNLUA_VERSION_MAJOR 由 NexusLinkExt 传入，
/// 这里只补链 UnLua，避免宏重定义。
/// </summary>
public class NexusLinkExtEditor : ModuleRules
{
	public NexusLinkExtEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "NexusLink", "NexusLinkExt", "Json", "JsonUtilities",
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
