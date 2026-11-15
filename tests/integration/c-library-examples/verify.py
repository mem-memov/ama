# @file
# @brief Check published example metadata and malformed SDK maps through the real model consumer.
# Contract: ../../../docs/implementation/c-library-examples.md.
"""Invoked by the assertion-based integration leaf after its four C hosts run."""

import copy
import json
from pathlib import Path
import sys
import tempfile

sys.path.insert(0, sys.argv[1])
import model


def rejected(operation):
    """Require explicit rejection rather than accepting an ambiguous authoring binding."""
    try:
        operation()
    except (ValueError, KeyError):
        return
    raise AssertionError("Malformed metadata was accepted")


root = Path(sys.argv[2]).resolve()
commands = json.loads((root / "commands.json").read_text())
assert all(set(row) == {"directory", "arguments"} for row in commands)
assert all(Path(row["directory"]).is_absolute() for row in commands)
consumers = [row["arguments"] for row in commands
             if any(str(arg).endswith(("observer.c", "standalone.c", "nested.c")) for arg in row["arguments"])]
assert len(consumers) == 3
assert all(not any(str(arg).endswith("/src") for arg in command) for command in consumers)
for component in ("child", "parent"):
    directory = root / component
    manifest = json.loads((directory / "component.json").read_text())
    assert manifest["format"] == "ama-c-build-1"
    assert len((directory / "symbols.txt").read_text().splitlines()) > 10
    for name in ("public.h", "library.a", "library.so"):
        assert (directory / name).stat().st_size > 0
    assert not (directory / "launcher.c").exists()
    assert model.native(directory / "native-model.json")
for scenario in ("standalone", "nested"):
    for linkage in ("static", "shared"):
        program = str(root / f"{scenario}-{linkage}")
        assert any(row["arguments"] == [program] for row in commands)
original = json.loads((root / "sdk-worker/native-model.json").read_text())
with tempfile.TemporaryDirectory() as temporary:
    path = Path(temporary) / "model.json"
    for mutate in (
        lambda data: data.update(format="unknown"),
        lambda data: data["modules"].append(copy.deepcopy(data["modules"][0])),
        lambda data: data["modules"][0]["namespace"].update(name="bad-name"),
        lambda data: data["modules"][0]["namespace"].update(key_hex=""),
        lambda data: data["modules"][0]["namespace"].update(key_hex="FF"),
        lambda data: data["modules"][0].update(interface="../escape.h"),
    ):
        malformed = copy.deepcopy(original)
        mutate(malformed)
        path.write_text(json.dumps(malformed))
        rejected(lambda: model.native(path))
    expected = model.native(root / "sdk-worker/native-model.json")
    altered = copy.deepcopy(expected)
    next(iter(altered.values()))["results"]["key_hex"] += "00"
    rejected(lambda: model.same_bindings(expected, altered))
    rejected(lambda: model.same_bindings(expected, {}))
    rejected(lambda: model.public({"format": "unknown"}, "BAD", "bad.h", path))
