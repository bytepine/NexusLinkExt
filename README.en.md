# NexusLinkExt — UnLua MCP Extension for NexusLink

**Debug UnLua scripts in Unreal Engine with AI.** NexusLinkExt is an optional extension for [NexusLink — Unreal Engine MCP](https://github.com/bytepine/NexusLink), providing runtime Lua debugging and Blueprint-to-Lua bindings.

[Download extension](https://github.com/bytepine/NexusLinkExt/releases) · [NexusLink setup](https://github.com/bytepine/NexusLink/blob/master/README.en.md#quick-start) · [Core plugin](https://github.com/bytepine/NexusLink)

Requires NexusLink and UnLua. Once installed, its Capabilities register into the same MCP server; discover them with `SearchCapabilities`. They are absent when this plugin is not loaded.

Today this repo covers **UnLua**: runtime Lua debugging and Blueprint `UnLuaInterface` binding. If the project has no UnLua, these Capabilities are not registered.

中文：[README.md](README.md).

## Modules

| Module | Type | Contents |
|------|------|------|
| `NexusLinkExt` | Runtime | 12 runtime Lua Capabilities and `FNexusLuaUtils` |
| `NexusLinkExtEditor` | Editor | `GetAssetLuaBinding`, `ManageAssetLuaBinding` |

## Install

Depends on [NexusLink](https://github.com/bytepine/NexusLink) and [UnLua](https://github.com/Tencent/UnLua). Place this plugin at `Plugins/NexusLinkExt` and enable `NexusLinkExt` in the `.uproject`.

The sample [NexusUnreal](https://github.com/bytepine/NexusUnreal) mounts it as a git submodule (`Plugins/NexusLinkExt`). Clone that project with `--recurse-submodules`.

Release zips are on [GitHub Releases](https://github.com/bytepine/NexusLinkExt/releases); the archive root is `NexusLinkExt/`. The source `VersionName` stays `0.0.0`. The version is the `VERSION` file and the `nexus-linkext-v*` tag. See [CONTRIBUTING.md](CONTRIBUTING.md).

Runtime Capabilities need PIE or a standalone Game, with UnLua ready. `HotReloadRuntimeLua` requires UnLua 2.x (1.x returns an error).

`EvalRuntimeLua` and `DofileRuntimeLua` carry the `dangerous` tag and are disabled by default. Enable them the same way as NexusLink's `ExecCommand`: the Dangerous Capability access mode in Editor Preferences, a session checkbox, or `-NexusEnableDangerousCaps`.

Do not send legacy keys. Script paths use `ScriptPath` (relative to `Content/Script/`). Table paths use `LuaPath`. `Path` / `FilePath` return `ArgInvalid`.

## Capabilities

### Runtime (`NexusLinkExt`)

| Capability | Description |
|---|---|
| `EvalRuntimeLua` | Execute a Lua snippet in PIE/Game and return stacked values. Dangerous; off by default |
| `DofileRuntimeLua` | Load and run a `.lua` file under `Content/Script/`. Dangerous; off by default |
| `GcRuntimeLua` | `Mode=collect\|stop\|restart\|count` |
| `HotReloadRuntimeLua` | Hot-reload on UnLua 2.x; 1.x returns an error |
| `SetRuntimeLua` | Assign a global or nested field by dot path (string/number/bool/null) |
| `GetRuntimeLuaEnv` | List keys of `_G` or a nested table; `NameFilter` + `Limit` |
| `GetRuntimeLuaValue` | Read by dot path; returns type and value |
| `GetRuntimeLuaLoaded` | List `package.loaded` |
| `GetRuntimeLuaStack` | Call stack; `Detail=locals\|upvalues\|all` |
| `GetRuntimeLuaMetatable` | Dump the OOP class table along `__index` |
| `GetRuntimeLuaObject` | Read the instance Lua table of a bound Actor/UObject |
| `GetRuntimeLuaMemory` | Lua heap (KB/bytes) |

### Editor (`NexusLinkExtEditor`)

| Capability | Description |
|---|---|
| `GetAssetLuaBinding` | Resolve the UnLua module bound to a Blueprint; returns `bBound` / `bFileExists` |
| `ManageAssetLuaBinding` | `Action=bind\|unbind` |

Parameter schemas come from `SearchCapabilities`.

## Authoring

New Capabilities still follow [CapabilitySpec](https://github.com/bytepine/NexusLink/blob/master/Resources/CapabilitySpec.md) §2.1.1 for the base class. Optional-plugin Capabilities belong in this repo, not in NexusLink.

- Runtime code goes in `NexusLinkExt`; editor assets go in `NexusLinkExtEditor`. New domains use `Private/Capabilities/<Domain>/`. Do not add another module.
- End the `.cpp` with `REGISTER_MCP_CAPABILITY`. Define `WITH_*` only in the module that first links that optional plugin.
- A dangerous Capability adds `dangerous` and `write` to `Out.Tags`. NexusLink treats that tag as the dangerous-capability switch. Do not register the name inside NexusLink.
- When the capability surface changes, edit `Resources/InitializeInstructions.SearchMode.md` and `InitializeInstructions.MultiTool.md`. Startup registers both fragments; shutdown unregisters them. Do not list these Capabilities in NexusLink's own handshake text.

The sample project's L1 suite is `Plugins/NexusLinkExtTestSuite` (Automation prefix `NexusLinkExt.`) and is not part of this repo.

## License

[MIT](LICENSE) © byteyang
