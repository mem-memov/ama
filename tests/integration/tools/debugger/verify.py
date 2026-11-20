"""Decode the actual shipped debugger client without parsing generated implementation artifacts."""
import json
import sys
from pathlib import Path

mode, name = sys.argv[1:]
text = Path(name).read_text()
if mode == "empty":
    assert text == ""
    sys.exit(0)
if mode == "existing":
    assert text == "handwritten response\n"
    sys.exit(0)
rows = [json.loads(line) for line in text.splitlines()]
assert all(row["schema"] == "ama.debugger.v1" for row in rows)

def root(row):
    """Select the actual nominal root, independently of enumeration order or slot identities."""
    return next(value for value in row["data"]["instances"] if value["type_name"] == '/test/fixture/v1.0.0/src/main')

if mode == "accepted":
    assert [row["command"] for row in rows] == ["run", "inspect", "inspect", "resume", "inspect", "resume", "inspect", "quit"]
    assert all(row["status"]["code"] == "0" for row in rows)
    assert [rows[index]["data"]["outcome"] for index in (0, 3, 5)] == ["yielded", "yielded", "completed"]
    assert rows[1]["data"] == rows[2]["data"]
    for index, boundary in ((1, "1"), (4, "2")):
        value = root(rows[index])
        assert value["resume_valid"] and not value["caller"]["attached"]
        assert value["resume_location"]["operation"] == value["resume_selector"]
        assert value["resume_location"]["path"]["utf8"].endswith('project/packages/test/fixture/v1.0.0/src/main')
        # The saved successor after the final yield is the next foreach check,
        # whose source is the collection header, rather than the yield itself.
        assert value["resume_location"]["line"] == "10"
        fields = {field["name"]: field for field in value["fields"]}
        assert set(fields) == {"мой список", "итог"}
        field = fields["мой список"]
        assert field["count"] == "2" and not field["acquired"]
        assert [item["position"] for item in field["elements"]] == ["0", "1"]
        assert field["elements"][0]["membership"] != field["elements"][1]["membership"]
        cursor, = value["cursors"]
        assert value["resume_selector"] == cursor["repeat"]
        assert cursor["binding_name"] == "текущий элемент" and cursor["boundary"] == boundary
        assert cursor["local"] and cursor["active"] and cursor["role_active"] and cursor["binding_roles"] == "1"
        assert cursor["value"] == field["elements"][int(boundary)-1]["target"]
        assert all(not item["native"]["registered"] for item in rows[index]["data"]["instances"])
    assert not root(rows[6])["resume_valid"] and not root(rows[6])["cursors"][0]["active"]
elif mode == "refused":
    assert len(rows) == 13
    for index in (0, 1, 2, 7, 10, 11):
        assert rows[index]["status"]["code"] != "0"
    assert rows[1]["data"] is None and rows[10]["data"] is None
    assert rows[2]["command"] == "invalid" and rows[2]["data"] is None
    assert rows[6]["data"] == rows[8]["data"]
    assert not root(rows[8])["resume_valid"]
    assert rows[9]["status"]["code"] == "0" and rows[12]["command"] == "quit"
elif mode == "final":
    assert len(rows) == 1 and rows[0]["command"] == "run" and rows[0]["data"]["outcome"] == "yielded"
elif mode == "failed":
    assert len(rows) == 3 and rows[0]["data"]["outcome"] == "failed" and rows[0]["status"]["code"] != "0"
    assert rows[1]["data"] is None and rows[1]["status"]["code"] != "0" and rows[2]["command"] == "quit"
else:
    raise AssertionError(mode)
