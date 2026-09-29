# NexusLinkExt

[NexusLink](https://github.com/bytepine/NexusLink) 的可选插件 Capability。安装后注册进同一套 MCP，用 `search_capabilities` 发现；未安装时这些名字不会出现。

当前包含 **UnLua**：运行时 Lua 调试，以及蓝图 `UnLuaInterface` 绑定。工程里没有 UnLua 时，本插件不注册这些 Capability。

English: [README.en.md](README.en.md).

## 模块

| 模块 | 类型 | 内容 |
|------|------|------|
| `NexusLinkExt` | Runtime | 12 个运行时 Lua Capability、`FNexusLuaUtils` |
| `NexusLinkExtEditor` | Editor | `get_asset_lua_binding`、`manage_asset_lua_binding` |

## 安装

依赖 [NexusLink](https://github.com/bytepine/NexusLink) 和 [UnLua](https://github.com/Tencent/UnLua)。将本插件放到项目 `Plugins/NexusLinkExt`，并在 `.uproject` 中启用 `NexusLinkExt`。

示例工程 [NexusUnreal](https://github.com/bytepine/NexusUnreal) 以 git 子模块挂载（`Plugins/NexusLinkExt`）。克隆示例工程时加 `--recurse-submodules`。

发布包在 [GitHub Releases](https://github.com/bytepine/NexusLinkExt/releases)，zip 内顶层为 `NexusLinkExt/`。源码 `VersionName` 保持 `0.0.0`，版本号以 `VERSION` 与 tag `nexus-linkext-v*` 为准。发版步骤见 [CONTRIBUTING.md](CONTRIBUTING.md)。

运行时 Capability 需要 PIE 或独立 Game，且 UnLua 已就绪。`hotreload_runtime_lua` 仅 UnLua 2.x（1.x 返回错误）。

`eval_runtime_lua` 与 `dofile_runtime_lua` 带 `dangerous` 标签，默认禁用。开闸方式与 NexusLink 的 `exec_command` 相同：Editor Preferences 里的危险 Capability 访问模式、会话勾选，或启动参数 `-NexusEnableDangerousCaps`。

入参不要用旧键：脚本路径是 `scriptPath`（相对 `Content/Script/`），表路径是 `luaPath`。`path` / `filePath` 会返回 `arg_invalid`。

## Capability

### 运行时（`NexusLinkExt`）

| Capability | 说明 |
|---|---|
| `eval_runtime_lua` | 在 PIE/Game 执行 Lua 片段，返回压栈值。危险，默认禁用 |
| `dofile_runtime_lua` | 从 `Content/Script/` 加载执行 `.lua`。危险，默认禁用 |
| `gc_runtime_lua` | `mode=collect\|stop\|restart\|count` |
| `hotreload_runtime_lua` | UnLua 2.x 热重载；1.x 返回错误 |
| `set_runtime_lua` | 按点路径写全局或嵌套字段（string/number/bool/null） |
| `get_runtime_lua_env` | 列出 `_G` 或嵌套表的键；`nameFilter` + `limit` |
| `get_runtime_lua_value` | 按点路径读取，返回类型和值 |
| `get_runtime_lua_loaded` | 列出 `package.loaded` |
| `get_runtime_lua_stack` | 调用栈；`detail=locals\|upvalues\|all` |
| `get_runtime_lua_metatable` | 沿 `__index` 转储 OOP 类表 |
| `get_runtime_lua_object` | 读取已绑定 Actor/UObject 的实例 Lua 表 |
| `get_runtime_lua_memory` | Lua 堆（KB/bytes） |

### 编辑器（`NexusLinkExtEditor`）

| Capability | 说明 |
|---|---|
| `get_asset_lua_binding` | 解析蓝图绑定的 UnLua 模块，返回 `bound` / `fileExists` |
| `manage_asset_lua_binding` | `action=bind\|unbind` |

参数以 `search_capabilities` 返回的 schema 为准。

## 编写

新 Capability 仍遵守 [CapabilitySpec](https://github.com/bytepine/NexusLink/blob/master/Resources/CapabilitySpec.md) §2.1.1 的基类选择。可选插件的 cap 写在本仓库，不写进 NexusLink。

- 运行时放 `NexusLinkExt`，编辑器资产放 `NexusLinkExtEditor`。新域用 `Private/Capabilities/<域>/`，不要再加模块
- `.cpp` 末尾 `REGISTER_MCP_CAPABILITY`。`WITH_*` 只在首次链接该可选插件的模块里定义
- 危险 cap 在 `Out.Tags` 加上 `dangerous` 和 `write`。NexusLink 按这个标签做默认禁用和确认，不用登记名字
- 改了能力面就改 `Resources/InitializeInstructions.SearchMode.md` 与 `InitializeInstructions.MultiTool.md`。模块启动时注册这两段，关闭时卸掉。不要改 NexusLink 的握手正文来列举这里的 cap

示例工程的 L1 在 `Plugins/NexusLinkExtTestSuite`（Automation 前缀 `NexusLinkExt.`），不随本仓库分发。

## License

[MIT](LICENSE) © byteyang
