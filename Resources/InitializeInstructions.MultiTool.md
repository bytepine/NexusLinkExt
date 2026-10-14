## UnLua (NexusLinkExt)

These tools appear in `tools/list` only while this plugin is loaded. Skip them when absent.

`HotReloadRuntimeLua` requires UnLua **2.x** (1.x returns an error). Runtime tools need PIE/Game and UnLua ready. Binding starts with `GetAssetLuaBinding`; if `bBound=false`, stop. Then `ManageAssetLuaBinding` (`Action=bind|unbind`).

`EvalRuntimeLua` and `DofileRuntimeLua` are tagged `dangerous` and off by default (same access mode as `ExecCommand`). Confirm mode requires `Reason`.

Table paths use `LuaPath` (not `Path`). Script paths use `ScriptPath`, relative to `Content/Script/` (not `FilePath`).
