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
    assert not value["success"] and value["status"] == "collision"
    assert value["error"]["code"] == "ambiguous-module"
    assert value["state"] == "rejected" and value["outcome_known"] and value["committed"] is False
    assert (base / 'root/packages/test/fixture/v1.0.0/src/old/a').exists() and not (base / 'target/packages/test/fixture/v1.0.0/src/new').exists()
    assert value["error"]
    raise SystemExit(0)
assert value["success"] and value["outcome_known"]
report = value["report"]
assert report["destination"]["utf8"] == str((base / 'root/packages/test/fixture/v1.0.0/src/new').resolve())
assert report["entry"] == '/test/fixture/v1.0.0/src/new/a' and report["rebuild_required"]
assert len(report["after_roots"]) == 2
identities = {row["before_identity"]: row for row in report["modules"]}
assert identities['/test/fixture/v1.0.0/src/old/a']["after_identity"] == '/test/fixture/v1.0.0/src/new/a'
assert identities['/test/fixture/v1.0.0/src/old/draft']["after_identity"] == '/test/fixture/v1.0.0/src/new/draft' and not identities['/test/fixture/v1.0.0/src/old/draft']["active"]
assert '/test/imports/v1.0.0/src/unreachable' in identities
if mode == "preview":
    assert value["command"] == "preview" and value["committed"] is False and value["journal"] is None
    assert len(value["edits"]) == 2
    assert (base / 'root/packages/test/fixture/v1.0.0/src/old/a').exists() and not (base / 'root/packages/test/fixture/v1.0.0/src/new').exists()
    assert (base / 'root/packages/test/fixture/v1.0.0/src/main').read_bytes() == pathlib.Path('project/root/packages/test/fixture/v1.0.0/src/main').read_bytes()
    assert (base / 'root/packages/test/fixture/v1.0.0/src/settings.library').read_bytes() == pathlib.Path('project/root/packages/test/fixture/v1.0.0/src/settings.library').read_bytes()
    assert (base / 'root/packages/test/fixture/v1.0.0/src/cache.bin').read_bytes() == b"outside generated cache"
else:
    assert value["committed"] is True and value["finished"] and value["state"] == "committed"
    assert not (base / 'root/packages/test/fixture/v1.0.0/src/old').exists() and (base / 'root/packages/test/fixture/v1.0.0/src/new/a').exists()
    for name in ['root/packages/test/fixture/v1.0.0/src/main', 'importers/packages/test/imports/v1.0.0/src/unreachable', 'root/packages/test/fixture/v1.0.0/src/settings.library']:
        original = pathlib.Path("project", name).read_bytes()
        assert (base / name).read_bytes() == original.replace(b'/test/fixture/v1.0.0/src/old/a', b'/test/fixture/v1.0.0/src/new/a')
    assert not (base / 'root/packages/test/fixture/v1.0.0/src/cache.bin').exists() and not (base / 'root/packages/test/fixture/v1.0.0/src/new/build.bin').exists()
    assert (base / 'root/packages/test/fixture/v1.0.0/src/new/asset.bin').read_bytes() == bytes([0, 255, 13, 10, 0, 7])
    for name in ["draft", "helper.c"]:
        assert (base / 'root/packages/test/fixture/v1.0.0/src/new' / name).read_bytes() == pathlib.Path('project/root/packages/test/fixture/v1.0.0/src/old', name).read_bytes()
    pathlib.Path(base / "receipt.txt").write_bytes(bytes.fromhex(value["journal"]["hex"]))
