# Contributing to NexusLinkExt

[中文](CONTRIBUTING.md) | English

For installation, see [README.en.md](README.en.md). Capability base classes and naming follow [NexusLink CapabilitySpec](https://github.com/bytepine/NexusLink/blob/master/Resources/CapabilitySpec.md) §2.1.1; extension-plugin conventions are in §2.1.2 of the same document.

## Documentation

All maintained documentation must be available in complete Chinese and English versions, in separate Chinese and English files with reciprocal links at the top. Update both languages together, including README files, contribution guides, changelogs, and instruction resources. Preserve command, path, API, and version identifiers. Update generated documentation through its source or generator.

## Releases

Follow the same workflow as NexusLink: **for daily work, only update `[Unreleased]` in `CHANGELOG.md`; do not change `VERSION` or create tags.** Keep `NexusLinkExt.uplugin`'s source `VersionName` at `0.0.0`; CI injects the release version during packaging.

Only when a release is explicitly requested:

1. For changes involving the Lua runtime / PIE, run `py Script/run_e2e.py --gui` in the sample project. Documentation-only or release-script-only changes do not require it.
2. Archive `[Unreleased]` as `[X.Y.Z] - YYYY-MM-DD` (or `[X.Y.Z-beta.N]`) and update `VERSION`. If a stable release has earlier beta versions in the same series, summarize their user-visible capabilities in `[X.Y.Z]` using short sentences; omit fixes to features newly introduced in that version. Preserve the beta sections.
3. Run `py scripts/extract_release_notes.py --version <version> --verify`.
4. `git commit` → `git tag -a nexus-linkext-v<version>` → `git push origin HEAD` + `git push origin nexus-linkext-v<version>`.

After the tag is pushed, `.github/workflows/release.yml` builds `nexus-mcp-ext-<ver>.zip` (with `NexusLinkExt/` as the top-level ZIP directory) and creates a GitHub Release from the matching CHANGELOG section. A version containing `-beta` becomes a Pre-release. Do not handwrite the Release body on the website.
