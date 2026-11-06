#!/usr/bin/env python3
"""Generates the console variable and command reference from the sources.

The owning file keeps the binding in a local named <name>_binding:

    auto& net_maxplayers_binding = cvar::bind_var(net_maxplayers, "net_maxplayers");
    net_maxplayers_binding.set_help("...");
    net_maxplayers_binding.set_flags(FCVAR_ARCHIVE);

The variable itself is declared in the same file. Bindings without the _binding local are
non-owners and are skipped, as is anything flagged FCVAR_HIDDEN.

docs/scripts/hooks.py runs this on every properdocs build.
"""

import argparse
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
DEFAULT_OUT = REPO_ROOT / "docs" / "reference" / "cvars.md"
REPO_URL = "https://github.com/untodesu/voxelius/blob/main"
SOURCE_DIRS = ("core", "shared", "client", "server")

SIDES = ("Client", "Server", "Shared")

BIND_VAR = re.compile(r'auto&\s+(\w+)_binding\s*=\s*cvar::bind_var\(\s*([\w:]+)\s*,\s*"([^"]+)"\s*\)')
BIND_CMD = re.compile(r'auto&\s+(\w+)_binding\s*=\s*cvar::bind_cmd\(\s*"([^"]+)"\s*\)')
DECLARATION = r"cvar::(Boolean|String|Number<\s*([\w:\s]+?)\s*>)\s+(?:\w+::)*{name}\s*[{{(](.*)[}})]\s*;"

TYPE_DEFAULTS = {"bool": "false", "string": '""'}


def string_literal(text):
    return bytes(text, "utf-8").decode("unicode_escape")


def binding_call(source, binding, method):
    match = re.search(rf'\b{binding}\.{method}\(((?:"(?:[^"\\]|\\.)*")|[^)]*)\)', source)
    return match.group(1) if match else None


def side_of(path):
    parts = path.parts

    if "client" in parts:
        return "Client"

    if "server" in parts:
        return "Server"

    return "Shared"


def number_literal(text):
    return re.sub(r"(?<=[\d.])[fFuU]+$", "", text.strip())


def parse_declaration(source, variable):
    name = variable.split("::")[-1]
    match = re.search(DECLARATION.format(name=re.escape(name)), source)

    if match is None:
        return ("?", "")

    kind, number_type, args = match.groups()
    args = [a.strip() for a in args.split(",")] if args.strip() else []

    if kind == "Boolean":
        return ("bool", args[0] if args else TYPE_DEFAULTS["bool"])

    if kind == "String":
        default = args[0] if args else TYPE_DEFAULTS["string"]
        return ("string", default)

    number_type = number_type.removeprefix("std::").removesuffix("_t")
    default = number_literal(args[0]) if args else "0"
    return (number_type, default)


def parse_flags(text):
    if not text:
        return []

    return [flag.strip().removeprefix("FCVAR_").lower() for flag in text.split("|") if flag.strip() != "FCVAR_NONE"]


def line_of(source, offset):
    return source.count("\n", 0, offset) + 1


def collect():
    variables = {}
    commands = {}

    for top in SOURCE_DIRS:
        for path in sorted((REPO_ROOT / top).rglob("*.cc")):
            source = path.read_text(encoding="utf-8")
            rel = path.relative_to(REPO_ROOT)

            for match in BIND_VAR.finditer(source):
                binding, variable, name = f"{match.group(1)}_binding", match.group(2), match.group(3)
                flags = parse_flags(binding_call(source, binding, "set_flags"))
                help_text = binding_call(source, binding, "set_help")
                type_name, default = parse_declaration(source, variable)

                entry = {
                    "name": name,
                    "type": type_name,
                    "default": default,
                    "flags": flags,
                    "help": string_literal(help_text[1:-1]) if help_text else "",
                    "side": side_of(rel),
                    "source": f"{rel.as_posix()}#L{line_of(source, match.start())}",
                }

                if "hidden" in flags:
                    continue

                if name in variables:
                    print(f"warning: {name} has two owners: {variables[name]['source']} and {entry['source']}", file=sys.stderr)

                variables[name] = entry

            for match in BIND_CMD.finditer(source):
                binding, name = f"{match.group(1)}_binding", match.group(2)
                flags = parse_flags(binding_call(source, binding, "set_flags"))
                help_text = binding_call(source, binding, "set_help")

                if "hidden" in flags:
                    continue

                commands[name] = {
                    "name": name,
                    "flags": flags,
                    "help": string_literal(help_text[1:-1]) if help_text else "",
                    "side": side_of(rel),
                    "source": f"{rel.as_posix()}#L{line_of(source, match.start())}",
                }

    return variables, commands


def cell(text):
    return str(text).replace("|", "\\|")


def name_link(entry):
    return f"[`{entry['name']}`]({REPO_URL}/{entry['source']})"


def code(text):
    return f"`{text}`" if text else ""


def code_list(items):
    return " ".join(code(item) for item in items)


def write_if_changed(path, text):
    if path.exists() and path.read_text(encoding="utf-8") == text:
        return False

    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")
    return True


def generate():
    variables, commands = collect()

    lines = [
        "# CVars",
        "",
        "<!-- Generated by docs/scripts/make-cvars.py. Do not edit, edit the sources and run the script. -->",
        "",
        "This page lists the cvars and commands that the engine and the game bind with a `<name>_binding` local.",
        "This page does not list debug and hidden cvars. To see all of them, use `list_vars` and `list_cmds` in the console.",
        "Each name links to the line in the sources that binds it.",
        "",
    ]

    for side in SIDES:
        side_vars = sorted((v for v in variables.values() if v["side"] == side), key=lambda v: v["name"])
        side_cmds = sorted((c for c in commands.values() if c["side"] == side), key=lambda c: c["name"])

        if not side_vars and not side_cmds:
            continue

        lines += [f"## {side}", ""]

        if side_vars:
            lines += ["### Variables", "", "| Name | Type | Default | Flags | Description |", "|---|---|---|---|---|"]

            for v in side_vars:
                lines.append(f"| {name_link(v)} | {code(v['type'])} | {code(cell(v['default']))} | {code_list(v['flags'])} | {cell(v['help'])} |")

            lines.append("")

        if side_cmds:
            lines += ["### Commands", "", "| Name | Flags | Description |", "|---|---|---|"]

            for c in side_cmds:
                lines.append(f"| {name_link(c)} | {code_list(c['flags'])} | {cell(c['help'])} |")

            lines.append("")

    return "\n".join(lines), len(variables), len(commands)


def main():
    parser = argparse.ArgumentParser(description="Generate the cvar reference from the sources")
    parser.add_argument("-out", type=Path, default=DEFAULT_OUT)
    args = parser.parse_args()

    text, variable_count, command_count = generate()
    write_if_changed(args.out, text)
    print(f"{variable_count} variables, {command_count} commands -> {args.out}")


if __name__ == "__main__":
    main()
