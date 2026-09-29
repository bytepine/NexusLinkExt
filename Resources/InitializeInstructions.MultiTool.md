## UnLua (NexusLinkExt)

These tools appear in `tools/list` only while this plugin is loaded. Skip them when absent.

`hotreload_runtime_lua` requires UnLua **2.x** (1.x returns an error). Runtime tools need PIE/Game and UnLua ready. Binding starts with `get_asset_lua_binding`; if `bound=false`, stop. Then `manage_asset_lua_binding` (`action=bind|unbind`).

`eval_runtime_lua` and `dofile_runtime_lua` are tagged `dangerous` and off by default (same access mode as `exec_command`). Confirm mode requires `reason`.

Table paths use `luaPath` (not `path`). Script paths use `scriptPath`, relative to `Content/Script/` (not `filePath`).
