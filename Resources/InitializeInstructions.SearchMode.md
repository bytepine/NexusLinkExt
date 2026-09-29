## UnLua (NexusLinkExt)

These capabilities exist only while this plugin is loaded. Discover them with `search_capabilities` (for example `lua binding`). Skip the names below when they are absent.

**Runtime** (PIE/Game, UnLua ready): `eval_runtime_lua` · `dofile_runtime_lua` · `gc_runtime_lua` · `hotreload_runtime_lua` · `set_runtime_lua` · `get_runtime_lua_env` · `get_runtime_lua_value` · `get_runtime_lua_loaded` · `get_runtime_lua_stack` · `get_runtime_lua_metatable` · `get_runtime_lua_object` · `get_runtime_lua_memory`. `hotreload_runtime_lua` requires UnLua **2.x** (1.x returns an error).

**Editor**: `get_asset_lua_binding` first; if `bound=false`, stop. Bind or unbind with `manage_asset_lua_binding` (`action=bind|unbind`).

**Dangerous** (tag `dangerous`, off by default, same access mode as `exec_command`): `eval_runtime_lua`, `dofile_runtime_lua`. Confirm mode requires `reason`.

**Parameters**: table paths use `luaPath` (not `path`). Script paths use `scriptPath`, relative to `Content/Script/` (not `filePath`).
