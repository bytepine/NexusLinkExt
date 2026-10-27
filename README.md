# NexusLinkExt — NexusLink 的 UnLua MCP 扩展

**让 AI 调试 Unreal Engine 项目中的 UnLua 脚本。** NexusLinkExt 是 [NexusLink — Unreal Engine MCP](https://github.com/bytepine/NexusLink) 的可选扩展，提供运行时 Lua 调试和蓝图 Lua 绑定能力。

[下载扩展](https://github.com/bytepine/NexusLinkExt/releases) · [NexusLink 安装指南](https://github.com/bytepine/NexusLink#快速开始) · [Fab 核心插件（NexusMCP）](https://www.fab.com/listings/60097027-d7a5-4f73-87f3-81edb1b6ef0b)

本扩展需配合 NexusLink 与 UnLua 使用。安装后注册进同一套 MCP，用 `SearchCapabilities` 发现；未安装时这些名字不会出现。

当前包含 **UnLua**：运行时 Lua 调试，以及蓝图 `UnLuaInterface` 绑定。工程里没有 UnLua 时，本插件不注册这些 Capability。

English: [README.en.md](README.en.md).

## 模块

| 模块 | 类型 | 内容 |
|------|------|------|
| `NexusLinkExt` | Runtime | 12 个运行时 Lua Capability、`FNexusLuaUtils` |
| `NexusLinkExtEditor` | Editor | `GetAssetLuaBinding`、`ManageAssetLuaBinding` |

## 安装

依赖 [NexusLink](https://github.com/bytepine/NexusLink) 和 [UnLua](https://github.com/Tencent/UnLua)。将本插件放到项目 `Plugins/NexusLinkExt`，并在 `.uproject` 中启用 `NexusLinkExt`。

示例工程 [NexusUnreal](https://github.com/bytepine/NexusUnreal) 以 git 子模块挂载（`Plugins/NexusLinkExt`）。克隆示例工程时加 `--recurse-submodules`。

发布包在 [GitHub Releases](https://github.com/bytepine/NexusLinkExt/releases)，zip 内顶层为 `NexusLinkExt/`。源码 `VersionName` 保持 `0.0.0`，版本号以 `VERSION` 与 tag `nexus-linkext-v*` 为准。发版步骤见 [CONTRIBUTING.md](CONTRIBUTING.md)。

运行时 Capability 需要 PIE 或独立 Game，且 UnLua 已就绪。`HotReloadRuntimeLua` 仅 UnLua 2.x（1.x 返回错误）。

`EvalRuntimeLua` 与 `DofileRuntimeLua` 带 `dangerous` 标签，默认禁用。开闸方式与 NexusLink 的 `ExecCommand` 相同：Editor Preferences 里的危险 Capability 访问模式、会话勾选，或启动参数 `-NexusEnableDangerousCaps`。

入参不要用旧键：脚本路径是 `ScriptPath`（相对 `Content/Script/`），表路径是 `LuaPath`。`Path` / `FilePath` 会返回 `ArgInvalid`。

## Capability

### 运行时（`NexusLinkExt`）

| Capability | 说明 |
|---|---|
| `EvalRuntimeLua` | 在 PIE/Game 执行 Lua 片段，返回压栈值。危险，默认禁用 |
| `DofileRuntimeLua` | 从 `Content/Script/` 加载执行 `.lua`。危险，默认禁用 |
| `GcRuntimeLua` | `Mode=collect\|stop\|restart\|count` |
| `HotReloadRuntimeLua` | UnLua 2.x 热重载；1.x 返回错误 |
| `SetRuntimeLua` | 按点路径写全局或嵌套字段（string/number/bool/null） |
| `GetRuntimeLuaEnv` | 列出 `_G` 或嵌套表的键；`NameFilter` + `Limit` |
| `GetRuntimeLuaValue` | 按点路径读取，返回类型和值 |
| `GetRuntimeLuaLoaded` | 列出 `package.loaded` |
| `GetRuntimeLuaStack` | 调用栈；`Detail=locals\|upvalues\|all` |
| `GetRuntimeLuaMetatable` | 沿 `__index` 转储 OOP 类表 |
| `GetRuntimeLuaObject` | 读取已绑定 Actor/UObject 的实例 Lua 表 |
| `GetRuntimeLuaMemory` | Lua 堆（KB/bytes） |

### 编辑器（`NexusLinkExtEditor`）

| Capability | 说明 |
|---|---|
| `GetAssetLuaBinding` | 解析蓝图绑定的 UnLua 模块，返回 `bBound` / `bFileExists` |
| `ManageAssetLuaBinding` | `Action=bind\|unbind` |

参数以 `SearchCapabilities` 返回的 schema 为准。

## 编写

新 Capability 仍遵守 [CapabilitySpec](https://github.com/bytepine/NexusLink/blob/master/Resources/CapabilitySpec.md) §2.1.1 的基类选择。可选插件的 cap 写在本仓库，不写进 NexusLink。

- 运行时放 `NexusLinkExt`，编辑器资产放 `NexusLinkExtEditor`。新域用 `Private/Capabilities/<域>/`，不要再加模块
- `.cpp` 末尾 `REGISTER_MCP_CAPABILITY`。`WITH_*` 只在首次链接该可选插件的模块里定义
- 危险 cap 在 `Out.Tags` 加上 `dangerous` 和 `write`。NexusLink 按这个标签做默认禁用和确认，不用登记名字
- 改了能力面就改 `Resources/InitializeInstructions.SearchMode.md` 与 `InitializeInstructions.MultiTool.md`。模块启动时注册这两段，关闭时卸掉。不要改 NexusLink 的握手正文来列举这里的 cap

示例工程的 L1 在 `Plugins/NexusLinkExtTestSuite`（Automation 前缀 `NexusLinkExt.`），不随本仓库分发。

## License

[MIT](LICENSE) © byteyang
