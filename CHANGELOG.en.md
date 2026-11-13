# Changelog — NexusLinkExt

[中文](CHANGELOG.md) | English

All changes follow [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).
Version numbers follow [Semantic Versioning](https://semver.org/).

---

## [Unreleased]

## [1.1.2] - 2026-11-10

### Fixed

- `ManageAssetLuaBinding` correctly writes and reads back the bound module name, supports explicit compilation and repeated updates, and protects dynamic connections and inherited logic.
- Lua query result fields consistently use PascalCase; add the `Lua inspect` discovery keyword and handshake guidance.
- Shipping builds do not read, register, or unregister MCP extension instructions.

## [1.1.1] - 2026-11-05

### Fixed

- `GetAssetLuaBinding` first searches for the bound file using UnLua's `GLuaSrcFullPath`, falls back to `Content/Script`, and returns `SearchedPaths`.
- `GetRuntimeLuaObject` reads instance tables from UnLua 2.x's `UnLua_ObjectMap` and 1.x's `ObjectMap`. When only a class table is available, it explicitly states that missing fields do not prove their absence from the instance.

## [1.1.0] - 2026-11-03

### Changed

- **Breaking:** Lua capability tool IDs and fields use PascalCase; boolean fields use the `b` prefix, matching NexusLink.
- Plugin descriptors no longer restrict modules to Win64 / Mac platform allowlists; other platforms still depend on compatibility with NexusLink, UnLua, and the engine.
- Chinese and English homepages explain the UnLua MCP extension's purpose, dependencies, and installation entry points.

### Fixed

- Lua-table-to-JSON conversion preserves stack balance at the 50-item truncation boundary and during nested conversion, preventing crashes.

## [1.1.0-beta.2] - 2026-10-28

> ⚠️ Pre-release; not for production use.

### Changed

- Clarify NexusLinkExt's role as an UnLua MCP extension, align the Chinese and English homepages and plugin display description, and add the core plugin's GitHub installation entry point.
- Remove `PlatformAllowList` and `WhitelistPlatforms` from Runtime / Editor modules. The plugin descriptor no longer restricts Win64 / Mac; other platforms still depend on NexusLink, UnLua, and engine compatibility.

## [1.1.0-beta.1] - 2026-10-23

> ⚠️ Pre-release; not for production use.

### Changed

- **Breaking:** Lua capability tool IDs and fields now use PascalCase, matching NexusLink. Boolean fields gain a `b` prefix.
- Align private capability class names with filenames, for example `FNexusEvalRuntimeLuaCapability`.

## [1.0.1] - 2026-10-13

### Fixed

- `manage_asset_lua_binding` returns an error for a null Blueprint.
- `hotreload_runtime_lua` returns an error when `GetMainState` is null.

## [1.0.1-beta.1] - 2026-10-12

> ⚠️ Pre-release; not for production use.

### Fixed

- fix(mcp): `manage_asset_lua_binding` records an error and returns for a null Blueprint instead of passing a null pointer to editor APIs.
- fix(mcp): `hotreload_runtime_lua` returns an error when `GetMainState` is null instead of passing a null `lua_State` to `lua_gc`.

## [1.0.0] - 2026-10-08

### Added

- feat(plugin): `NexusLinkExt` (Runtime) and `NexusLinkExtEditor` (Editor) host the UnLua capabilities and `FNexusLuaUtils`.
- docs: The README lists 14 UnLua capabilities, installation steps, and dangerous-capability guidance, linking back to NexusLink. Development rules cover the existing two modules, `REGISTER_MCP_CAPABILITY`, dangerous tags, and startup registration of handshake fragments through `FNexusInstructionRegistry`. Sample-project L1 tests live in `NexusLinkExtTestSuite`, outside this repository.

### Changed

- chore(plugin): Align releases with NexusLink: `VERSION`, `scripts/build_unreal.py`, and `scripts/extract_release_notes.py`; the `nexus-linkext-v*` tag triggers CI to build `nexus-mcp-ext-<ver>.zip`. Keep source `VersionName` at `0.0.0`.
