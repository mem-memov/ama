# @file
# @brief Independently decode the C writer fixture and verify byte-exact control, Unicode, binary and numeric transport.
import json
import pathlib
import sys

value = json.loads(pathlib.Path(sys.argv[1]).read_text(encoding="utf-8"))
assert value["text"] == "".join(map(chr, range(32))) + '\"\\/Имя'
assert value["maximum"] == sys.argv[2]
assert value["bytes"] == {"utf8": None, "hex": bytes(range(256)).hex()}
assert value["empty"] == {"utf8": "", "hex": ""}
assert value["flag"] is True
assert value["absent"] is None
assert value["boundary"] == chr(0x10FFFF)
