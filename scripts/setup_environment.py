#!/usr/bin/env python3
"""Configures the local VSCode environment for one of the two C/C++ engines.

    python scripts/setup_environment.py clangd [--preset NAME]
    python scripts/setup_environment.py intellisense

Everything this script writes is specific to the machine it runs on and is ignored by git,
except .vscode/c_cpp_properties.json, which holds only paths relative to the workspace.

.vscode/settings.json is edited in place rather than replaced:
  * engine settings (ENGINE_SETTINGS) are owned by this script: they are overwritten on every
    run, and the ones that belong only to the other environment are removed;
  * common settings (DEFAULT_SETTINGS) are written only while the key is absent, so a value
    changed by hand survives; delete the key and re-run to get the default back;
  * every other key is left alone.

A file is rewritten only when its content actually changes. JSON comments do not survive a
rewrite, so a file that has them is first copied to *.<timestamp>.bak.
"""

import argparse
import copy
import json
import re
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
VSCODE = ROOT / ".vscode"

DEFAULT_SETTINGS = {
    "editor.formatOnSave": True,
    "C_Cpp.autoAddFileAssociations": False,
    "C/C++ Include Guard.Auto Include Guard Insertion For New File": False,
    "C/C++ Include Guard.Auto Update Include Guard": False,
    "[c][cpp]": {"editor.rulers": [140]},
    "[markdown]": {"editor.rulers": [80]},
    "[json][jsonc][jsonl]": {"editor.tabSize": 2, "editor.formatOnSave": False},
}

# Include-path and define defaults for the Microsoft C/C++ extension. The clangd environment
# needs no c_cpp_properties.json: it reads build/compile_commands.json instead.
CPP_DEFINES = ["TRACY_ENABLE", "NOMINMAX", "_CRT_SECURE_NO_WARNINGS", "_USE_MATH_DEFINES"]

CLANGD_EXTENSION = "llvm-vs-code-extensions.vscode-clangd"
CPPTOOLS_EXTENSION = "ms-vscode.cpptools"

ENGINE_SETTINGS = {
    "clangd": {
        # ms-vscode.cpptools has to stay installed for its debugger, only its analysis is off.
        "C_Cpp.intelliSenseEngine": "disabled",
        "C_Cpp.formatting": "disabled",
        "C_Cpp.codeAnalysis.runAutomatically": False,
        "clangd.enable": True,
        # No --compile-commands-dir: clangd then looks for a database upwards from each file,
        # so build/compile_commands.json (copied there by setup_compilation_database) serves
        # the whole project.
        "clangd.arguments": [
            "--background-index",
            "--background-index-priority=low",
            "--clang-tidy",
            "--completion-style=detailed",
            "--all-scopes-completion",
            "--header-insertion=never",
            "--pch-storage=memory",
        ],
        "clangd.detectExtensionConflicts": False,
        "clangd.onConfigChanged": "restart",
        "[c]": {"editor.defaultFormatter": CLANGD_EXTENSION},
        "[cpp]": {"editor.defaultFormatter": CLANGD_EXTENSION},
    },
    "intellisense": {
        "C_Cpp.intelliSenseEngine": "default",
        "clangd.enable": False,
        "[c]": {"editor.defaultFormatter": CPPTOOLS_EXTENSION},
        "[cpp]": {"editor.defaultFormatter": CPPTOOLS_EXTENSION},
    },
}

_JSONC_COMMENT = re.compile(r'"(?:\\.|[^"\\])*"|//[^\n]*|/\*.*?\*/', re.DOTALL)
_JSONC_TRAILING_COMMA = re.compile(r'"(?:\\.|[^"\\])*"|,(?=\s*[}\]])')


def _keep_strings(match):
    return match.group() if match.group().startswith('"') else ""


def relative(path):
    return path.relative_to(ROOT).as_posix()


def backup(path):
    target = path.with_name(path.name + time.strftime(".%Y%m%d-%H%M%S.bak"))
    target.write_bytes(path.read_bytes())
    return target


def load(path):
    """Reads a JSON file the way VSCode does. Returns (data, True if a rewrite loses something)."""
    if not path.exists():
        return {}, False

    text = path.read_text(encoding="utf-8-sig")
    stripped = _JSONC_COMMENT.sub(_keep_strings, text)

    try:
        data = json.loads(_JSONC_TRAILING_COMMA.sub(_keep_strings, stripped))
    except json.JSONDecodeError as error:
        print(f"  warning  {relative(path)} cannot be parsed ({error}); starting from an empty one")
        return {}, True

    if not isinstance(data, dict):
        print(f"  warning  {relative(path)} is not a JSON object; starting from an empty one")
        return {}, True

    return data, stripped != text


def save(path, data, original, lossy):
    if path.exists() and data == original:
        print(f"  ok       {relative(path)} is up to date")
        return

    if lossy:
        print(f"  backup   {relative(path)} -> {relative(backup(path))}")

    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8", newline="\r\n")
    print(f"  write    {relative(path)}")


def force(target, values, trail=()):
    """Overwrites every leaf of values in target."""
    for key, value in values.items():
        if isinstance(value, dict):
            if not isinstance(target.get(key), dict):
                target[key] = {}
            force(target[key], value, trail + (key,))
        elif key not in target or target[key] != value:
            target[key] = copy.deepcopy(value)
            print(f"  set      {' > '.join(trail + (key,))}")


def seed(target, values, trail=()):
    """Writes the leaves of values that target does not have yet."""
    for key, value in values.items():
        if isinstance(value, dict):
            if isinstance(target.setdefault(key, {}), dict):
                seed(target[key], value, trail + (key,))
        elif key not in target:
            target[key] = copy.deepcopy(value)
            print(f"  default  {' > '.join(trail + (key,))}")


def prune(target, values, keep, trail=()):
    """Removes the leaves of values from target, except for the ones that keep has too."""
    for key, value in values.items():
        if key not in target:
            continue
        if isinstance(value, dict):
            if isinstance(target[key], dict):
                nested = keep.get(key)
                prune(target[key], value, nested if isinstance(nested, dict) else {}, trail + (key,))
                if not target[key]:
                    del target[key]
        elif key not in keep:
            del target[key]
            print(f"  remove   {' > '.join(trail + (key,))}")


def setup_settings(environment):
    path = VSCODE / "settings.json"
    settings, lossy = load(path)
    original = copy.deepcopy(settings)

    for other, values in ENGINE_SETTINGS.items():
        if other != environment:
            prune(settings, values, ENGINE_SETTINGS[environment])

    force(settings, ENGINE_SETTINGS[environment])
    seed(settings, DEFAULT_SETTINGS)
    save(path, settings, original, lossy)


def dependency_includes():
    """Every dependency keeps its public headers in deps/<name>/include; tracy differs."""
    directories = sorted(directory for directory in (ROOT / "deps").glob("*/include") if directory.is_dir())
    tracy = ROOT / "deps" / "tracy" / "public" / "tracy"

    paths = [relative(directory) for directory in directories]

    if tracy.is_dir():
        paths.append(relative(tracy))

    return sorted(paths)


def sdl3_includes():
    """The Windows presets download SDL3 into build/<preset>/_deps at configure time."""
    presets = json.loads((ROOT / "CMakePresets.json").read_text(encoding="utf-8-sig"))
    paths = []

    for preset in presets.get("configurePresets", []):
        binary_dir = preset.get("binaryDir", "")

        if not preset.get("name", "").startswith("windows-") or "${sourceDir}/" not in binary_dir:
            continue

        paths.append(binary_dir.split("${sourceDir}/", 1)[1] + "/_deps/download_sdl3-src/include")

    return paths


def setup_cpp_properties():
    """Configurations are matched by name: ours are replaced, the ones added by hand are kept."""
    path = VSCODE / "c_cpp_properties.json"
    properties, lossy = load(path)
    original = copy.deepcopy(properties)

    include_path = [f"${{workspaceFolder}}/{directory}" for directory in sdl3_includes() + dependency_includes()]
    configuration = {
        "name": "Voxelius",
        "cppStandard": "c++20",
        "cStandard": "c99",
        "defines": list(CPP_DEFINES),
        "includePath": include_path + ["${workspaceFolder}", "${default}"],
    }

    properties.setdefault("version", 4)
    if not isinstance(properties.get("configurations"), list):
        properties["configurations"] = []

    existing = properties["configurations"]
    for index, candidate in enumerate(existing):
        if isinstance(candidate, dict) and candidate.get("name") == configuration["name"]:
            existing[index] = configuration
            break
    else:
        existing.append(configuration)

    save(path, properties, original, lossy)


def setup_compilation_database(preset):
    """CMake presets configure into build/<preset>/, but clangd only looks for a database in the
    directories above a file and in their build/ subdirectory, so the database of one preset is
    copied to build/compile_commands.json. The copy goes stale on reconfigure: re-run the script.
    Only the Ninja and Makefile generators write a database; Visual Studio writes none."""
    target = ROOT / "build" / "compile_commands.json"

    if preset:
        source = ROOT / "build" / preset / "compile_commands.json"
    else:
        candidates = sorted(ROOT.glob("build/*/compile_commands.json"), key=lambda path: path.stat().st_mtime)
        source = candidates[-1] if candidates else None

    if source is None:
        print("  warning  no compilation database under build/*/compile_commands.json")
        print("           configure a preset with Ninja, which writes one, and re-run:")
        print("             cmake -S . -B build/clangd -G Ninja -DCMAKE_BUILD_TYPE=Debug \\")
        print("                   -DCMAKE_EXPORT_COMPILE_COMMANDS=ON")
        return False

    if not source.is_file():
        print(f"  warning  {relative(source)} does not exist; configure the preset first:")
        print(f"             cmake --preset {preset}")
        return False

    data = source.read_bytes()
    if target.is_file() and target.read_bytes() == data:
        print(f"  ok       {relative(target)} is up to date ({relative(source.parent)})")
    else:
        target.write_bytes(data)
        print(f"  write    {relative(target)} from {relative(source.parent)}")
    return True


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("environment", choices=sorted(ENGINE_SETTINGS))
    parser.add_argument(
        "--preset",
        help="clangd: the CMake preset whose compilation database to use (default: the most recently configured one)",
    )
    args = parser.parse_args()
    environment = args.environment

    print(f"Environment '{environment}' -> {VSCODE}")
    setup_settings(environment)

    if environment == "clangd":
        if not setup_compilation_database(args.preset):
            print()
            print("warning: clangd cannot parse the project without a compilation database.")
    else:
        setup_cpp_properties()

    print()
    print("Done. Reload the VSCode window (Developer: Reload Window) to apply the settings.")


if __name__ == "__main__":
    main()
