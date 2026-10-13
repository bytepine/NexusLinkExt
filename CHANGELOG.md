# Changelog — NexusLinkExt

所有变更记录遵循 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.0.0/) 格式。
版本号遵循 [语义化版本控制](https://semver.org/lang/zh-CN/)。

---

## [Unreleased]

## [1.0.1] - 2026-10-13

### Fixed

- `manage_asset_lua_binding` 在蓝图为空时返回 error
- `hotreload_runtime_lua` 在 `GetMainState` 为空时返回 error

## [1.0.1-beta.1] - 2026-10-12

> ⚠️ Pre-release，非生产环境使用。

### Fixed

- fix(mcp): `manage_asset_lua_binding` 在蓝图为空时写入 error 并返回，不再把空指针交给编辑器接口
- fix(mcp): `hotreload_runtime_lua` 在 `GetMainState` 为空时返回 error，不再把空 `lua_State` 交给 `lua_gc`

## [1.0.0] - 2026-10-08

### Added

- feat(plugin): `NexusLinkExt`（Runtime）与 `NexusLinkExtEditor`（Editor），承接 UnLua Capability 与 `FNexusLuaUtils`
- docs: README 列出 14 个 UnLua Capability、安装方式与危险 cap 说明，并链回 NexusLink。编写规则：已有两模块、`REGISTER_MCP_CAPABILITY`、危险标签、启动时向 `FNexusInstructionRegistry` 注册握手片段。示例工程 L1 在 `NexusLinkExtTestSuite`，不在本仓

### Changed

- chore(plugin): 发版与 NexusLink 对齐。`VERSION`、`scripts/build_unreal.py`、`scripts/extract_release_notes.py`；tag `nexus-linkext-v*` 触发 CI 打出 `nexus-mcp-ext-<ver>.zip`。源码 `VersionName` 保持 `0.0.0`
