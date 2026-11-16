"""Decode real tool results and publish only this private fixture's exclusively owned document transaction."""
import json
import os
import sys
from pathlib import Path

mode, base, extra = sys.argv[1:]
base = Path(base).resolve()
original_file = Path("generated/original-inventory.json")
renamed_file = Path("generated/renamed-inventory.json")
logical_file = Path("generated/logical-inventory.json")


def inventory(root):
    """Capture every owned regular file, including opaque native helpers, drafts and binary assets."""
    return {str(path.relative_to(root)): path.read_bytes().hex()
            for path in sorted(root.rglob("*")) if path.is_file()}


def save(path, value):
    """Retain an independent exact-byte oracle for the next explicitly ordered C-harness step."""
    path.write_text(json.dumps(value, ensure_ascii=False, sort_keys=True))


def decode():
    """Require a complete standard JSON value from the already successful real command."""
    return json.load(sys.stdin)


if mode == "snapshot":
    save(original_file, inventory(base))
elif mode == "editor":
    value = decode()
    assert value["schema"] == "ama.editor.v1" and value["state"] == "complete"
    symbol = value["point"]["definition"]["symbol"]
    assert symbol["owner"] == ".palette" and symbol["name"]["utf8"] == "шаг"
    assert inventory(base) == json.loads(original_file.read_text())
elif mode == "rename":
    value = decode()
    assert value["schema"] == "ama.rename.v1"
    before = json.loads(original_file.read_text())
    # The C harness owns the complete domain, including unchanged sources/native
    # dependencies, and admits no observer or writer until this callback returns.
    # This is a test document publisher, not a production filesystem-edit service.
    assert inventory(base) == before
    changed = {"app/palette", "app/old/sequence", "importers/unused"}
    prepared = []
    observed = set()
    try:
        for index, record in enumerate(value["files"]):
            path = Path(os.fsdecode(bytes.fromhex(record["path"]["hex"])))
            relative = str(path.resolve().relative_to(base))
            assert relative in changed and relative not in observed
            observed.add(relative)
            information = path.stat()
            assert information.st_dev == int(record["device"]) and information.st_ino == int(record["inode"])
            original = bytes.fromhex(record["original"]["hex"])
            replacement = bytes.fromhex(record["replacement"]["hex"])
            assert original == bytes.fromhex(before[relative]) == path.read_bytes()
            expected = original.replace("\nшаг\n".encode(), "\nразмер шага\n".encode()) if relative == "app/palette" else b"".join(
                " .palette.размер шага\n".encode() if line == " .palette.шаг\n".encode() else line
                for line in original.splitlines(keepends=True))
            assert replacement == expected and replacement != original, relative
            staged = path.with_name(".client-stage-" + str(index))
            with staged.open("xb") as stream:
                stream.write(replacement)
            os.chmod(staged, information.st_mode & 0o777)
            prepared.append((staged, path))
        assert observed == changed
        # All preparation and exact original checks precede publication. No reader
        # can observe an intermediate document set in this fixture's exclusive scope.
        for staged, path in prepared:
            os.replace(staged, path)
    finally:
        for staged, _ in prepared:
            staged.unlink(missing_ok=True)
    after = inventory(base)
    assert set(after) == set(before)
    assert {name for name in before if before[name] != after[name]} == changed
    save(renamed_file, after)
elif mode in ("preview", "logical", "container"):
    value = decode()
    assert value["schema"] == "ama.relocation.v1" and value["success"] and value["outcome_known"]
    report = value["report"]
    assert report["entry"] == ".new.sequence" and report["rebuild_required"]
    assert {row["kind"] for row in report["after_roots"]} == {"project", "standard"}
    if mode != "container":
        before = json.loads(renamed_file.read_text())
        mapping = {row["before_identity"]: row["after_identity"] for row in report["modules"]}
        assert mapping[".old.sequence"] == ".new.sequence" and mapping["..integer"] == "..integer"
        if mode == "preview":
            assert value["command"] == "preview" and value["committed"] is False and value["journal"] is None
            assert inventory(base) == before and Path("generated/before/program").exists()
            sys.exit(0)
        expected = {}
        for name, content in before.items():
            final_name = "app/new/" + name[len("app/old/"):] if name.startswith("app/old/") else name
            data = bytes.fromhex(content)
            if name in {"app/main", "importers/unused"}:
                data = data.replace(b"\n.old.sequence\n", b"\n.new.sequence\n")
            if name == "app/settings.library":
                data = data.replace(b" | .old.sequence | ", b" | .new.sequence | ")
            expected[final_name] = data.hex()
        assert inventory(base) == expected
        assert not Path("generated/before/program").exists()
        save(logical_file, expected)
    else:
        assert value["edits"] == []
        assert all(row["before_identity"] == row["after_identity"] for row in report["modules"])
        assert inventory(base) == json.loads(logical_file.read_text())
        assert not Path("generated/исходная область").exists() and not Path("generated/after-logical/program").exists()
        assert {Path(row["path"]["utf8"]) for row in report["after_roots"]} == {
            base / "app", base / "importers", base / "library"}
    assert value["committed"] and value["finished"] and value["state"] == "committed"
    Path(extra).write_bytes(bytes.fromhex(value["journal"]["hex"]))
elif mode == "debug":
    rows = [json.loads(line) for line in Path(extra).read_text().splitlines()]
    assert len(rows) == 8 and all(row["status"]["code"] == "0" for row in rows)
    assert [rows[index]["data"]["outcome"] for index in (0, 3, 5)] == ["yielded", "yielded", "completed"]
    assert rows[1]["data"] == rows[2]["data"]
    for index, boundary in ((1, "1"), (4, "2")):
        root = next(item for item in rows[index]["data"]["instances"] if item["type_name"] in {".old.sequence", ".new.sequence"})
        field, = root["fields"]
        cursor, = root["cursors"]
        assert field["name"] == "числа" and field["count"] == "2" and len(field["elements"]) == 2
        assert cursor["binding_name"] == "элемент" and cursor["boundary"] == boundary
        assert root["resume_selector"] == cursor["repeat"] and not root["caller"]["attached"]
        expected_source = base / "app" / ("old" if root["type_name"] == ".old.sequence" else "new") / "sequence"
        assert Path(root["resume_location"]["path"]["utf8"]).resolve() == expected_source
        assert any(item["type_name"] == "..integer" and item["native_opaque"] for item in rows[index]["data"]["instances"])
        assert any(item["type_name"] == "..integer" and item["recorded_operation"] != "0"
                   and item["recorded_location"] is not None
                   and Path(item["recorded_location"]["path"]["utf8"]).resolve() == base / "library/integer"
                   for item in rows[index]["data"]["instances"])
elif mode == "final":
    assert inventory(base) == json.loads(logical_file.read_text())
    assert Path("generated/after-container/program").is_file()
else:
    raise AssertionError(mode)
