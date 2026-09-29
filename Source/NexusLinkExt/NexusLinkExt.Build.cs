// Copyright byteyang. All Rights Reserved.

using UnrealBuildTool;

/// <summary>
/// NexusLinkExt（Runtime）：可选插件的运行时 Capability。当前是 UnLua
/// （eval/dofile/gc/hotreload 与 get/set_runtime_lua_*）和 FNexusLuaUtils。
/// WITH_UNLUA / UNLUA_VERSION_MAJOR 只在本模块定义，经 PublicDefinitions 传给依赖方。
/// </summary>
public class NexusLinkExt : ModuleRules
{
	public NexusLinkExt(ReadOnlyTargetRules Target) : base(Target)
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
		ApplyUnLuaVersionDefines(this, bHasUnLua, SearchDirs);
	}

	/// <summary>从 UnLua.uplugin 的 VersionName 读主版本号，写入 UNLUA_VERSION_MAJOR。</summary>
	static void ApplyUnLuaVersionDefines(ModuleRules Module, bool bHasUnLua, System.Collections.Generic.List<string> SearchDirs)
	{
		if (!bHasUnLua) { Module.PublicDefinitions.Add("UNLUA_VERSION_MAJOR=0"); return; }

		int major = 1;
		foreach (string dir in SearchDirs)
		{
			if (major > 1) break;
			try
			{
				foreach (string path in System.IO.Directory.GetFiles(dir, "UnLua.uplugin", System.IO.SearchOption.AllDirectories))
				{
					string json = System.IO.File.ReadAllText(path);
					int vi = json.IndexOf("\"VersionName\"");
					if (vi >= 0)
					{
						int qi = json.IndexOf("\"", json.IndexOf(":", vi) + 1);
						if (qi >= 0)
						{
							int ds = qi + 1, de = ds;
							while (de < json.Length && char.IsDigit(json[de])) de++;
							if (de > ds) int.TryParse(json.Substring(ds, de - ds), out major);
						}
					}
					break;
				}
			}
			catch (System.Exception) { }
		}
		Module.PublicDefinitions.Add("UNLUA_VERSION_MAJOR=" + major);
	}
}
