## UnLua (NexusLinkExt)

[English](InitializeInstructions.MultiTool.md) | 中文

仅在此插件已加载时，这些工具才会出现在 `tools/list` 中。缺失时跳过。

`HotReloadRuntimeLua` 要求 UnLua **2.x**（1.x 返回错误）。运行时工具需要 PIE/Game 且 UnLua 已就绪。绑定前先检查 `GetAssetLuaBinding`；若尚未绑定，不要猜测 Lua 文件路径。`ManageAssetLuaBinding(Operations=[{Action:Bind|Unbind,ModuleName:...}])` 写入常量 GetModuleName 返回引脚，并拒绝已有连线或继承逻辑。使用 `bCompile=true` 编译后，验证 `GetAssetLuaBinding.bBound` 与 `ModuleName`；仅有 `bModuleNameWritten` 不能证明已编译或文件存在。

`EvalRuntimeLua` 和 `DofileRuntimeLua` 标为 `Dangerous`，默认关闭（访问模式与 `ExecCommand` 相同）。Confirm 模式要求 `Reason`。

表路径使用 `LuaPath`（不是 `Path`）。脚本路径使用 `ScriptPath`，相对 `Content/Script/`（不是 `FilePath`）。
