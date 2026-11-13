## UnLua (NexusLinkExt)

English | [中文](InitializeInstructions.MultiTool.zh.md)

These tools appear in `tools/list` only while this plugin is loaded. Skip them when absent.

`HotReloadRuntimeLua` requires UnLua **2.x** (1.x returns an error). Runtime tools need PIE/Game and UnLua ready. Inspect `GetAssetLuaBinding` before binding; if unbound, do not guess Lua file paths. `ManageAssetLuaBinding(Operations=[{Action:Bind|Unbind,ModuleName:...}])` writes constant GetModuleName return pins and rejects linked/inherited logic. Compile with `bCompile=true`, then verify `GetAssetLuaBinding.bBound` and `ModuleName`; `bModuleNameWritten` alone does not confirm compilation or file existence.

`EvalRuntimeLua` and `DofileRuntimeLua` are tagged `Dangerous` and off by default (same access mode as `ExecCommand`). Confirm mode requires `Reason`.

Table paths use `LuaPath` (not `Path`). Script paths use `ScriptPath`, relative to `Content/Script/` (not `FilePath`).
