"""
build_unreal.py — NexusLinkExt UE 插件打包（独立仓）

用法:
    python scripts/build_unreal.py --version <版本号> [--engine-version <引擎版>] [--output <输出目录>]

说明:
    1. 复制插件源码到临时目录（排除 scripts/、release/ 等仓级文件）
    2. 注入 NexusLinkExt.uplugin VersionName；可选 --engine-version 覆盖 EngineVersion
    3. 输出 release/nexus-mcp-ext-<ver>[-ue<engine>].zip（zip 内顶层为 NexusLinkExt/）
"""

from __future__ import annotations

import argparse
import json
import os
import shutil
import sys
import tempfile
import zipfile

if sys.stdout.encoding and sys.stdout.encoding.lower() != "utf-8":
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
if sys.stderr.encoding and sys.stderr.encoding.lower() != "utf-8":
    sys.stderr.reconfigure(encoding="utf-8", errors="replace")

PLUGIN_DIR_NAME = "NexusLinkExt"
UPLUGIN_NAME = "NexusLinkExt.uplugin"
ZIP_PREFIX = "nexus-mcp-ext"

EXCLUDE_DIRS = {
    "binaries",
    "intermediate",
    "deriveddatacache",
    "saved",
    "build",
    "release",
    "scripts",
    ".github",
    ".pytest_cache",
    ".vs",
    ".idea",
    ".git",
}

EXCLUDE_ROOT_FILES = {
    "readme.md",
    "readme.en.md",
    "changelog.md",
    "version",
    "license",
    ".gitignore",
    ".gitattributes",
}

EXCLUDE_EXTS = {
    ".pdb", ".obj", ".lib", ".exp", ".dll", ".so", ".dylib",
    ".log", ".tmp", ".bak",
}

_LFS_POINTER_PREFIX = b"version https://git-lfs.github.com/spec/v1"
_LFS_POINTER_MAX_BYTES = 1024


def repo_root() -> str:
    return os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def is_lfs_pointer(path: str) -> bool:
    try:
        if os.path.getsize(path) > _LFS_POINTER_MAX_BYTES:
            return False
        with open(path, "rb") as f:
            head = f.read(len(_LFS_POINTER_PREFIX) + 8)
    except OSError:
        return False
    return head.startswith(_LFS_POINTER_PREFIX)


def assert_no_lfs_pointers(plugin_dir: str) -> None:
    bad: list[str] = []
    for root, dirs, files in os.walk(plugin_dir):
        dirs[:] = [d for d in dirs if d.lower() not in EXCLUDE_DIRS]
        for filename in files:
            abs_file = os.path.join(root, filename)
            rel = os.path.relpath(abs_file, plugin_dir).replace("\\", "/")
            is_root = "/" not in rel
            if should_exclude(rel, is_root):
                continue
            if is_lfs_pointer(abs_file):
                bad.append(rel)
    if bad:
        listed = "\n  - ".join(bad)
        raise RuntimeError(
            "检测到 Git LFS pointer（未拉取实体文件），拒绝打包。\n"
            "  请在 CI checkout 设置 lfs: true，或本地执行 git lfs pull。\n"
            f"  - {listed}"
        )


def patch_uplugin(uplugin_path: str, version: str, engine_version: str | None = None) -> None:
    with open(uplugin_path, encoding="utf-8") as f:
        data = json.load(f)
    data["VersionName"] = version
    data["IsBetaVersion"] = "-beta" in version
    if engine_version is not None:
        data["EngineVersion"] = engine_version
    with open(uplugin_path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent="\t", ensure_ascii=False)
        f.write("\n")


def should_exclude(rel_path: str, is_root_file: bool) -> bool:
    norm = rel_path.replace("\\", "/")
    parts = norm.split("/")
    if is_root_file and len(parts) == 1 and parts[0].lower() in EXCLUDE_ROOT_FILES:
        return True
    for part in parts[:-1]:
        if part.lower() in EXCLUDE_DIRS:
            return True
    _, ext = os.path.splitext(norm)
    return ext.lower() in EXCLUDE_EXTS


def build_zip(
    plugin_dir: str,
    version: str,
    output_dir: str,
    engine_version: str | None = None,
) -> str:
    os.makedirs(output_dir, exist_ok=True)
    zip_name = f"{ZIP_PREFIX}-{version}"
    if engine_version is not None:
        zip_name += f"-ue{engine_version}"
    output_path = os.path.join(output_dir, f"{zip_name}.zip")

    assert_no_lfs_pointers(plugin_dir)

    with tempfile.TemporaryDirectory() as tmpdir:
        tmp_plugin = os.path.join(tmpdir, PLUGIN_DIR_NAME)
        shutil.copytree(
            plugin_dir,
            tmp_plugin,
            ignore=shutil.ignore_patterns(
                "Binaries",
                "Intermediate",
                "release",
                "scripts",
                ".github",
                ".git",
                ".pytest_cache",
            ),
        )

        tmp_uplugin = os.path.join(tmp_plugin, UPLUGIN_NAME)
        if os.path.isfile(tmp_uplugin):
            patch_uplugin(tmp_uplugin, version, engine_version)

        with zipfile.ZipFile(output_path, "w", zipfile.ZIP_DEFLATED) as zf:
            for root, dirs, files in os.walk(tmp_plugin):
                dirs[:] = [d for d in dirs if d.lower() not in EXCLUDE_DIRS]
                for filename in files:
                    abs_file = os.path.join(root, filename)
                    rel_in_plugin = os.path.relpath(abs_file, tmp_plugin)
                    rel_in_zip = os.path.join(PLUGIN_DIR_NAME, rel_in_plugin)
                    is_root = "/" not in rel_in_plugin.replace("\\", "/")
                    if should_exclude(rel_in_plugin, is_root):
                        continue
                    zf.write(abs_file, rel_in_zip.replace("\\", "/"))

    return output_path


def main() -> int:
    parser = argparse.ArgumentParser(description="打包 NexusLinkExt UE 插件")
    parser.add_argument("--version", required=True)
    parser.add_argument(
        "--engine-version",
        default=None,
        help="覆盖 NexusLinkExt.uplugin EngineVersion",
    )
    parser.add_argument("--output", default=None, help="默认 <repo>/release/")
    args = parser.parse_args()

    root = repo_root()
    output_dir = args.output or os.path.join(root, "release")

    if not os.path.isfile(os.path.join(root, UPLUGIN_NAME)):
        print(f"[ERROR] 非 NexusLinkExt 仓根目录: {root}", file=sys.stderr)
        return 1

    ev = args.engine_version
    print(f"[build] NexusLinkExt v{args.version}" + (f" (EngineVersion {ev})" if ev else ""))
    try:
        path = build_zip(root, args.version, output_dir, ev)
    except Exception as e:
        print(f"[ERROR] {e}", file=sys.stderr)
        return 1
    print(f"[OK] {path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
