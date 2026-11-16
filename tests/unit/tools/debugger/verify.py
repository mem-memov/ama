"""Independently decode actual context observations and four explicit native adapter views."""
import json
import sys
from pathlib import Path

rows = [json.loads(line) for line in Path(sys.argv[1]).read_text().splitlines()]
if len(sys.argv) == 3:
    assert sys.argv[2] == "failed-preview" and len(rows) == 2
    assert rows[0]["status"]["code"] != "0" and rows[0]["data"]["context"]["status"]["code"] == "0"
    root = next(value for value in rows[0]["data"]["instances"] if value["reference"]["type"] == "2")
    assert root["native"]["value"] is not None and root["native"]["status"] == rows[0]["status"]
    assert root["last_refusal"]["code"] == "0" and rows[1]["status"]["code"] == "0"
    sys.exit(0)
assert len(rows) == 5 and rows[-1]["command"] == "quit"
values = []
baseline = None
for row in rows[:-1]:
    assert row["schema"] == "ama.debugger.v1" and row["status"]["code"] == "0"
    data = row["data"]
    assert data["context"]["status"]["code"] == "0"
    root = next(value for value in data["instances"] if value["reference"]["type"] == "2")
    assert len(root["fields"]) == 6 and root["cursors"] == []
    assert all(not value["native"]["registered"] for value in data["instances"] if value is not root)
    assert root["native"]["registered"] and root["native"]["status"]["code"] == "0"
    values.append(root["native"]["value"])
    root["native"]["value"] = None
    if baseline is None:
        baseline = data
    assert data == baseline
assert values[0]["kind"] == "0" and values[0]["flag"] and values[0]["bytes"]["hex"] == ""
assert values[1]["kind"] == "1" and values[1]["negative"] and values[1]["little_endian"]
assert values[1]["bytes"]["hex"] == "ff01" and values[1]["bytes"]["utf8"] is None
assert values[2]["kind"] == "2" and values[2]["bytes"]["utf8"] == "А\0\n"
assert values[3]["kind"] == "3" and values[3]["bytes"] == {"utf8": None, "hex": "00ff0d0a"}
