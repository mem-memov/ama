# @file
# @brief Bind example C aliases only from checked public/native model metadata.
# Contract: ../../../docs/implementation/c-library-examples.md.
"""Exact metadata consumers; do not reconstruct hashes or inspect C declarations."""

import json
import re
from pathlib import Path


def identifier(value):
    """Require a complete C identifier before placing a compiler-provided name in code."""
    if not isinstance(value, str) or not re.fullmatch(r"[A-Za-z_][A-Za-z_0-9]*", value):
        raise ValueError("Invalid generated C identifier")
    return value


def native(path):
    """Read one versioned SDK map and reject duplicate modules or malformed full keys."""
    document = json.loads(Path(path).read_text(encoding="utf-8"))
    if document["format"] != "ama-native-model-1":
        raise ValueError("Unsupported native authoring model")
    modules = {}
    for row in document["modules"]:
        if row["module"] in modules:
            raise ValueError("Duplicate native module in authoring model")
        for role in ("namespace", "parameters", "results"):
            identifier(row[role]["name"])
            key = bytes.fromhex(row[role]["key_hex"])
            if not key or key.hex() != row[role]["key_hex"]:
                raise ValueError("Invalid complete native identity")
        for field in ("interface", "begin", "end"):
            if not re.fullmatch(r"[A-Za-z_][A-Za-z_0-9.-]*", row[field]):
                raise ValueError("Invalid generated SDK basename")
        modules[row["module"]] = row
    return modules


def authoring(row, tag, path, helpers=()):
    """Write exact include/view aliases and explicitly selected native helper namespaces."""
    lines = [f'#define {tag}_{key.upper()}_HEADER "{row[key]}"'
             for key in ("interface", "begin", "end")]
    lines.extend(f'#define {tag}_{role.upper()} {row[role]["name"]}'
                 for role in ("parameters", "results"))
    lines.extend(f'#define {macro}(n) {helper["namespace"]["name"]}_ ## n'
                 for macro, helper in helpers)
    Path(path).write_text("\n".join(lines) + "\n", encoding="utf-8")


def public(document, tag, header, path):
    """Use exact declared export/schema names; public headers remain the ABI authority."""
    if document["format"] != "ama-c-build-1":
        raise ValueError("Unsupported component manifest")
    prefix = identifier(document["identity"])
    exports = {row["name"]: row for row in document["exports"]}
    schemas = {row["name"]: row for row in document["schemas"]}
    lines = [f'#include "{header}"', f'#define {tag}(n) {prefix}_ ## n',
             f'#define {tag}_EXPORT(n) {identifier(exports["run"]["identity"])}_ ## n',
             f'#define {tag}_YIELDED {prefix.upper()}_YIELDED',
             f'#define {tag}_FAILED {prefix.upper()}_FAILED']
    for name in ("text", "condition", "failure"):
        if name in schemas:
            lines.append(f'#define {tag}_{name.upper()}(n) {identifier(schemas[name]["identity"])}_ ## n')
    Path(path).write_text("\n".join(lines) + "\n", encoding="utf-8")


def same_bindings(expected, actual):
    """Compare full SDK identity records, never candidate hash spellings alone."""
    for module, row in expected.items():
        if actual.get(module) != row:
            raise ValueError(f"Native authoring binding changed while building {module}")
