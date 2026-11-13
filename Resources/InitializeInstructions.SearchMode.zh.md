## UnLua (NexusLinkExt)

[English](InitializeInstructions.SearchMode.md) | 中文

仅在此插件已加载时，这些能力才存在。使用 `SearchCapabilities` 发现它们（例如搜索 `lua binding`）。下列名称缺失时跳过。

**运行时**（PIE/Game，UnLua 已就绪）：`EvalRuntimeLua` · `DofileRuntimeLua` · `GcRuntimeLua` · `HotReloadRuntimeLua` · `SetRuntimeLua` · `GetRuntimeLuaEnv` · `GetRuntimeLuaValue` · `GetRuntimeLuaLoaded` · `GetRuntimeLuaStack` · `GetRuntimeLuaMetatable` · `GetRuntimeLuaObject` · `GetRuntimeLuaMemory`。`HotReloadRuntimeLua` 要求 UnLua **2.x**（1.x 返回错误）。

**编辑器**：编辑前检查 `GetAssetLuaBinding`。尚未绑定时，不要猜测 Lua 文件路径。通过 `ManageAssetLuaBinding(Operations=[{Action:Bind|Unbind,ModuleName:...}])` 绑定/解绑。绑定会写入常量 GetModuleName 返回引脚，并拒绝已有连线或继承逻辑；使用 `bCompile=true`，然后验证 `GetAssetLuaBinding.bBound` 和 `ModuleName`。仅有 `bModuleNameWritten` 不能证明已编译或文件存在。

**危险能力**（标为 `Dangerous`，默认关闭，访问模式与 `ExecCommand` 相同）：`EvalRuntimeLua`、`DofileRuntimeLua`。Confirm 模式要求 `Reason`。

**参数**：表路径使用 `LuaPath`（不是 `Path`）。脚本路径使用 `ScriptPath`，相对 `Content/Script/`（不是 `FilePath`）。
