# @file
# @brief Independently decode command outcomes without confusing preview, commit, rollback or unknown recovery state.
import json
import pathlib
import sys

value = json.loads(pathlib.Path(sys.argv[2]).read_text())
assert value["schema"] == "ama.relocation.v1"
if sys.argv[1] == "preview":
    assert value["command"] == "preview" and value["success"] and value["committed"] is False
    assert value["outcome_known"] and value["journal"] is None and value["edits"]
    assert any(m["before_identity"] == "/test/fixture/v1.0.0/src/old/a" and m["after_identity"] == "/test/fixture/v1.0.0/src/new/a" for m in value["report"]["modules"])
    assert value["report"]["rebuild_required"]
elif sys.argv[1] == "recover":
    assert value["command"] == "recover" and value["success"] and value["outcome_known"]
    assert value["committed"] and value["finished"] and value["state"] == "committed"
elif sys.argv[1] == "unknown":
    assert not value["success"] and not value["outcome_known"] and value["committed"] is None
    assert value["status"] == "invalid-journal" and value["error"]
else:
    raise AssertionError(sys.argv[1])
