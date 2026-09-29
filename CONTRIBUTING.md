# 为 NexusLinkExt 做贡献

用户安装见 [README.md](README.md)。Capability 基类与命名见 [NexusLink CapabilitySpec](https://github.com/bytepine/NexusLink/blob/master/Resources/CapabilitySpec.md) §2.1.1；扩展插件约定见同文 §2.1.2。

## 发版

与 NexusLink 同一套：**日常只写 `CHANGELOG.md` 的 `[Unreleased]`，不改 `VERSION`、不打 tag。** 源码里 `NexusLinkExt.uplugin` 的 `VersionName` 保持 `0.0.0`，打包时由 CI 注入。

仅在明确发版时：

1. 含 Lua 运行时 / PIE 的改动，在示例工程跑 `py Script/run_e2e.py --gui`。只改文档或发版脚本则不强制。
2. 归档 `[Unreleased]` → `[X.Y.Z] - YYYY-MM-DD`（或 `[X.Y.Z-beta.N]`），更新 `VERSION`。正式版若已有同系列 beta，把用户可见能力汇总进 `[X.Y.Z]`（短句；本版本新功能上的 fix 不写）。beta 段落保留。
3. `py scripts/extract_release_notes.py --version <版本号> --verify`
4. `git commit` → `git tag -a nexus-linkext-v<版本号>` → `git push origin HEAD` + `git push origin nexus-linkext-v<版本号>`

push tag 后 `.github/workflows/release.yml` 打包 `nexus-mcp-ext-<ver>.zip`（zip 内顶层为 `NexusLinkExt/`），并用 CHANGELOG 对应段落创建 GitHub Release。版本号含 `-beta` 时为 Pre-release。不要在网页手写 Release 正文。
