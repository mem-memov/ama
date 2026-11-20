# @file
# @brief Check source-tool wire observations independently with the standard JSON decoder and original filesystem bytes.
import json
import pathlib
import sys

mode, filename = sys.argv[1:3]
value = json.loads(pathlib.Path(filename).read_text(encoding="utf-8"))
if mode == "editor":
    assert value["schema"] == "ama.editor.v1" and value["state"] == "complete"
    definition = value["point"]["definition"]
    assert definition["symbol"]["name"]["utf8"] == "старое имя"
    assert definition["declaration"]["start"] == "4"
    assert definition["declaration"]["line"] == "2" and definition["declaration"]["column"] == "1"
    assert value["point"]["operation"]["namespace"] == ".cell"
    assert any(item["name"]["utf8"] == "старое имя" for item in value["point"]["completions"]["items"])
    comments = [item for item in value["highlights"]["items"] if item["comment"]]
    assert len(comments) == 1 and comments[0]["category"] is None
    assert bytes.fromhex(comments[0]["bytes"]["hex"]).endswith("старое имя  ".encode())
elif mode == "rename":
    assert value["schema"] == "ama.rename.v1" and len(value["files"]) == 1
    file = value["files"][0]
    original = pathlib.Path("generated/project/main").read_bytes()
    assert bytes.fromhex(file["original"]["hex"]) == original
    expected = original.replace("старое имя\r\n".encode(), "новое имя\r\n".encode()).replace("старое имя !".encode(), "новое имя !".encode())
    assert bytes.fromhex(file["replacement"]["hex"]) == expected
    assert len(file["edits"]) == 2 and value["declaration"]["start"] == "4"
    assert value["declaration"]["path"]["utf8"].endswith("/main")
    assert file["replacement"]["utf8"].encode() == expected
elif mode == "binding":
    binding = value["point"]["binding"]
    assert binding["field"]["owner"] == ".worker" and binding["field"]["role"] == "parameter"
    assert binding["field"]["name"]["utf8"] == "argument" and binding["position"] == "0"
    assert binding["type"] == ".cell" and binding["intrinsic_name"] is None
elif mode == "intrinsic":
    call = value["point"]["operation"]
    assert call["kind"] == "list-method" and call["namespace"] is None
    assert call["intrinsic"]["method"] == "append" and call["element_type"] == ".cell"
    assert call["intrinsic"]["parameters"] == [{"name": "source", "reference": True, "type": ".cell"}]
    assert call["intrinsic"]["results"][0]["type"] == ".cell"
    assert value["point"]["definition"] is None
elif mode == "related":
    error = value["diagnostics"][0]
    assert value["state"] == "invalid" and error["related"]
    assert error["location"]["span"] is not None and error["related"][0]["location"]["span"] is not None
elif mode == "draft":
    assert value["state"] == "draft" and value["signature"]["available"] is False
    assert value["highlights"]["complete"] and value["point"]["definition"] is None
    assert any(item["payload"] and item["payload"]["utf8"] == "#  literal  " for item in value["highlights"]["items"])
elif mode == "invalid":
    assert value["state"] == "invalid" and value["diagnostics"]
    assert value["signature"]["available"] is False and value["point"]["definition"] is None
elif mode == "static-interface":
    assert value["state"] == "complete" and value["signature"]["available"]
    assert value["signature"]["parameters"] == []
    results = value["signature"]["results"]
    assert {item["name"]["utf8"] for item in results} == {"...left", "...right"}
    assert all(item["role"] == "type-result" for item in results)
    declaration = value["point"]["definition"]
    assert declaration["declaration"]["path"]["utf8"].endswith("/box")
    assert declaration["symbol"]["name"]["utf8"] == "...slot"
    assert declaration["symbol"]["role"] == "type-parameter"
    assert declaration["type"] is None
elif mode == "capture-source":
    assert value["state"] == "complete" and value["signature"]["available"]
    parameters = value["signature"]["parameters"]
    assert len(parameters) == 1 and parameters[0]["name"]["utf8"] == "value"
    assert parameters[0]["type"] is None and parameters[0]["position"] == "0"
    declaration = value["point"]["definition"]
    assert declaration["symbol"]["name"]["utf8"] == "value"
    assert declaration["symbol"]["type"] is None and declaration["type"] is None
    assert declaration["declaration"]["path"]["utf8"].endswith("/box")
elif mode == "static-related":
    assert value["state"] == "invalid" and not value["signature"]["available"]
    assert value["point"]["definition"] is None
    related = value["diagnostics"][0]["related"]
    callers = [item for item in related if item["role"] == "ordinary declaration requiring this static export"]
    assert len(callers) == 1
    assert callers[0]["location"]["span"]["path"]["utf8"].endswith("/main")
    assert callers[0]["location"]["span"]["line"] == "4"
else:
    raise AssertionError(mode)
