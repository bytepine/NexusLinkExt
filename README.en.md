# NexusLinkExt

Optional-plugin Capabilities for [NexusLink](https://github.com/bytepine/NexusLink). Once installed they register into the same MCP server; discover them with `search_capabilities`. They are absent when this plugin is not loaded.

Today this repo covers **UnLua**: runtime Lua debugging and Blueprint `UnLuaInterface` binding. If the project has no UnLua, these Capabilities are not registered.

中文：[README.md](README.md).

## Modules

| Module | Type | Contents |
|------|------|------|
| `NexusLinkExt` | Runtime | 12 runtime Lua Capabilities and `FNexusLuaUtils` |
| `NexusLinkExtEditor` | Editor | `get_asset_lua_binding`, `manage_asset_lua_binding` |

## Install

Depends on [NexusLink](https://github.com/bytepine/NexusLink) and [UnLua](https://github.com/Tencent/UnLua). Place this plugin at `Plugins/NexusLinkExt` and enable `NexusLinkExt` in the `.uproject`.

The sample [NexusUnreal](https://github.com/bytepine/NexusUnreal) mounts it as a git submodule (`Plugins/NexusLinkExt`). Clone that project with `--recurse-submodules`.

Runtime Capabilities need PIE or a standalone Game, with UnLua ready. `hotreload_runtime_lua` requires UnLua 2.x (1.x returns an error).

`eval_runtime_lua` and `dofile_runtime_lua` carry the `dangerous` tag and are disabled by default. Enable them the same way as NexusLink's `exec_command`: the Dangerous Capability access mode in Editor Preferences, a session checkbox, or `-NexusEnableDangerousCaps`.

Do not send legacy keys. Script paths use `scriptPath` (relative to `Content/Script/`). Table paths use `luaPath`. `path` / `filePath` return `arg_invalid`.

## Capabilities

### Runtime (`NexusLinkExt`)

| Capability | Description |
|---|---|
| `eval_runtime_lua` | Execute a Lua snippet in PIE/Game and return stacked values. Dangerous; off by default |
| `dofile_runtime_lua` | Load and run a `.lua` file under `Content/Script/`. Dangerous; off by default |
| `gc_runtime_lua` | `mode=collect\|stop\|restart\|count` |
| `hotreload_runtime_lua` | Hot-reload on UnLua 2.x; 1.x returns an error |
| `set_runtime_lua` | Assign a global or nested field by dot path (string/number/bool/null) |
| `get_runtime_lua_env` | List keys of `_G` or a nested table; `nameFilter` + `limit` |
| `get_runtime_lua_value` | Read by dot path; returns type and value |
| `get_runtime_lua_loaded` | List `package.loaded` |
| `get_runtime_lua_stack` | Call stack; `detail=locals\|upvalues\|all` |
| `get_runtime_lua_metatable` | Dump the OOP class table along `__index` |
| `get_runtime_lua_object` | Read the instance Lua table of a bound Actor/UObject |
| `get_runtime_lua_memory` | Lua heap (KB/bytes) |

### Editor (`NexusLinkExtEditor`)

| Capability | Description |
|---|---|
| `get_asset_lua_binding` | Resolve the UnLua module bound to a Blueprint; returns `bound` / `fileExists` |
| `manage_asset_lua_binding` | `action=bind\|unbind` |

Parameter schemas come from `search_capabilities`. The sample project's L1 suite is `Plugins/NexusLinkExtTestSuite` (Automation prefix `NexusLinkExt.`) and is not part of this repo.

## License

[MIT](LICENSE) © byteyang
