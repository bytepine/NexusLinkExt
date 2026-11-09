## UnLua (NexusLinkExt)

These capabilities exist only while this plugin is loaded. Discover them with `SearchCapabilities` (for example `lua binding`). Skip the names below when they are absent.

**Runtime** (PIE/Game, UnLua ready): `EvalRuntimeLua` · `DofileRuntimeLua` · `GcRuntimeLua` · `HotReloadRuntimeLua` · `SetRuntimeLua` · `GetRuntimeLuaEnv` · `GetRuntimeLuaValue` · `GetRuntimeLuaLoaded` · `GetRuntimeLuaStack` · `GetRuntimeLuaMetatable` · `GetRuntimeLuaObject` · `GetRuntimeLuaMemory`. `HotReloadRuntimeLua` requires UnLua **2.x** (1.x returns an error).

**Editor**: inspect `GetAssetLuaBinding` before editing. If unbound, do not guess Lua file paths. Bind/unbind with `ManageAssetLuaBinding(Operations=[{Action:Bind|Unbind,ModuleName:...}])`. Bind writes constant GetModuleName return pins and rejects linked/inherited logic; use `bCompile=true`, then verify `GetAssetLuaBinding.bBound` and `ModuleName`. `bModuleNameWritten` alone does not confirm compilation or file existence.

**Dangerous** (tag `Dangerous`, off by default, same access mode as `ExecCommand`): `EvalRuntimeLua`, `DofileRuntimeLua`. Confirm mode requires `Reason`.

**Parameters**: table paths use `LuaPath` (not `Path`). Script paths use `ScriptPath`, relative to `Content/Script/` (not `FilePath`).
