# @file
# @brief Independently decode full diagnostic attribution and original Unicode source locations.
import json
import pathlib
import sys

value = json.loads(pathlib.Path(sys.argv[1]).read_text(encoding="utf-8"))
assert value["message"] == "Static mismatch" and value["warning"] is False
notes = value["related"]
assert [item["role"] for item in notes] == ["original related", "root", "caller"]
assert [item["name"]["utf8"] for item in notes] == [".definition", ".root", ".caller"]
assert all(item["location"]["span"]["path"]["utf8"] == "данные-имя" for item in notes)
assert [item["location"]["span"]["line"] for item in notes] == ["1", "2", "2"]
