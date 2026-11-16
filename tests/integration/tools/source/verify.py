# @file
# @brief Independently check actual command JSON against original maintained source bytes and whole-workspace rename scope.
import json
import pathlib
import sys

value = json.load(sys.stdin)
mode = sys.argv[1]
if mode == "editor":
    assert value["state"] == "complete" and value["module"] == ".palette"
    assert value["point"]["definition"]["symbol"]["name"]["utf8"] == "общее имя"
    assert value["point"]["definition"]["symbol"]["owner"] == ".palette"
    assert value["signature"]["constants"][0]["name"]["utf8"] == "общее имя"
else:
    assert value["schema"] == "ama.rename.v1"
    expected = set() if mode == "noop" else {"main", "palette", "unused"}
    assert {pathlib.Path(f["path"]["utf8"]).name for f in value["files"]} == expected
    for f in value["files"]:
        name = pathlib.Path(f["path"]["utf8"]).name
        original = pathlib.Path("project", name).read_bytes()
        assert bytes.fromhex(f["original"]["hex"]) == original
        assert bytes.fromhex(f["replacement"]["hex"]) == original.replace("общее имя".encode(), "новое общее имя".encode())
assert pathlib.Path("project/palette").read_text() == "@=\nобщее имя\n.cell\n"
assert "общее имя" in pathlib.Path("project/helper.c").read_text()
assert pathlib.Path("project/draft").read_bytes() == "unfinished\r\n|общее имя  \r\n".encode()
