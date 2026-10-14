## UnLua (NexusLinkExt)

These capabilities exist only while this plugin is loaded. Discover them with `SearchCapabilities` (for example `lua binding`). Skip the names below when they are absent.

**Runtime** (PIE/Game, UnLua ready): `EvalRuntimeLua` · `DofileRuntimeLua` · `GcRuntimeLua` · `HotReloadRuntimeLua` · `SetRuntimeLua` · `GetRuntimeLuaEnv` · `GetRuntimeLuaValue` · `GetRuntimeLuaLoaded` · `GetRuntimeLuaStack` · `GetRuntimeLuaMetatable` · `GetRuntimeLuaObject` · `GetRuntimeLuaMemory`. `HotReloadRuntimeLua` requires UnLua **2.x** (1.x returns an error).

**Editor**: `GetAssetLuaBinding` first; if `bBound=false`, stop. Bind or unbind with `ManageAssetLuaBinding` (`Action=bind|unbind`).

**Dangerous** (tag `dangerous`, off by default, same access mode as `ExecCommand`): `EvalRuntimeLua`, `DofileRuntimeLua`. Confirm mode requires `Reason`.

**Parameters**: table paths use `LuaPath` (not `Path`). Script paths use `ScriptPath`, relative to `Content/Script/` (not `FilePath`).
