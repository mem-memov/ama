# @file
# @brief Independently validate exact relocation reports, original preservation and applied filesystem/configuration effects.
import json
import pathlib
import sys

mode, base = sys.argv[1:3]
base = pathlib.Path(base)
value = json.load(sys.stdin)
assert value["schema"] == "ama.relocation.v1"
if mode == "collision":
    assert not value["success"] and value["status"] == "invalid-reference"
    assert value["error"]["code"] == "ambiguous-module"
    assert value["state"] == "rejected" and value["outcome_known"] and value["committed"] is False
    assert (base / "root/old/a").exists() and not (base / "target/new").exists()
    assert value["error"]
    raise SystemExit(0)
assert value["success"] and value["outcome_known"]
report = value["report"]
assert report["destination"]["utf8"] == str((base / "root/new").resolve())
assert report["entry"] == ".new.a" and report["rebuild_required"]
assert len(report["after_roots"]) == 2
identities = {row["before_identity"]: row for row in report["modules"]}
assert identities[".old.a"]["after_identity"] == ".new.a"
assert identities[".old.draft"]["after_identity"] == ".new.draft" and not identities[".old.draft"]["active"]
assert ".unreachable" in identities
if mode == "preview":
    assert value["command"] == "preview" and value["committed"] is False and value["journal"] is None
    assert len(value["edits"]) == 2
    assert (base / "root/old/a").exists() and not (base / "root/new").exists()
    assert (base / "root/main").read_bytes() == pathlib.Path("project/root/main").read_bytes()
    assert (base / "root/settings.library").read_bytes() == pathlib.Path("project/root/settings.library").read_bytes()
    assert (base / "root/cache.bin").read_bytes() == b"outside generated cache"
else:
    assert value["committed"] is True and value["finished"] and value["state"] == "committed"
    assert not (base / "root/old").exists() and (base / "root/new/a").exists()
    for name in ["root/main", "importers/unreachable", "root/settings.library"]:
        original = pathlib.Path("project", name).read_bytes()
        assert (base / name).read_bytes() == original.replace(b".old.a", b".new.a")
    assert not (base / "root/cache.bin").exists() and not (base / "root/new/build.bin").exists()
    assert (base / "root/new/asset.bin").read_bytes() == bytes([0, 255, 13, 10, 0, 7])
    for name in ["draft", "helper.c"]:
        assert (base / "root/new" / name).read_bytes() == pathlib.Path("project/root/old", name).read_bytes()
    pathlib.Path(base / "receipt.txt").write_bytes(bytes.fromhex(value["journal"]["hex"]))
