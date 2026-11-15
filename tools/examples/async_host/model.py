# @file
# @brief Derive dynamic C consumer bindings from the checked component manifest and symbol inventory.
# Contract: ../../../docs/implementation/asynchronous-host.md.
"""No generated C declaration or guessed hash is used as an ABI authority."""

import re
from pathlib import Path


def identifier(value):
    """Require a complete producer-issued C identifier before emitting source."""
    if not isinstance(value, str) or not re.fullmatch(r"[A-Za-z_][A-Za-z_0-9]*", value):
        raise ValueError("Invalid generated C identifier")
    return value


def named(rows):
    """Reject duplicate public names instead of silently replacing an earlier declaration."""
    result = {}
    for row in rows:
        name = row["name"]
        if not isinstance(name, str) or not name or name in result:
            raise ValueError("Invalid or duplicate public name")
        result[name] = identifier(row["identity"])
    return result


def consumer(document, component, source):
    """Emit typed loader slots from the actual public inventory and matching header."""
    if document["format"] != "ama-c-build-1":
        raise ValueError("Unsupported component manifest")
    exports = named(document["exports"])
    schemas = named(document["schemas"])
    symbols = [identifier(line) for line in (component / "symbols.txt").read_text(encoding="utf-8").splitlines()]
    if not symbols or len(symbols) != len(set(symbols)):
        raise ValueError("Empty or duplicate public symbol inventory")
    lines = ["#include <dlfcn.h>", "#include <assert.h>", "#include <stdio.h>", "#include <string.h>"]
    lines.extend(f"#define {name} (*{name}_slot)" for name in symbols)
    lines.extend([f'#include "{component / "public.h"}"',
                  f'#define API(n) {identifier(document["identity"])}_ ## n',
                  f'#define RUN(n) {exports["run"]}_ ## n', f'#define INVALID(n) {exports["invalid"]}_ ## n',
                  f'#define TEXT(n) {schemas["text"]}_ ## n', f'#define FLAG(n) {schemas["flag"]}_ ## n',
                  "static void *image;", "static void load_image(void) {",
                  f'image=dlopen("{component / "library.so"}",RTLD_NOW|RTLD_LOCAL);',
                  'if(image==NULL) fprintf(stderr,"dlopen: %s\\n",dlerror());', "assert(image!=NULL);"])
    for name in symbols:
        lines.extend([f'{{ void *address=dlsym(image,"{name}"); assert(address!=NULL);',
                      f'_Static_assert(sizeof({name}_slot)==sizeof(address),"supported POSIX pointer size");',
                      f"memcpy(&{name}_slot,&address,sizeof(address)); }}"])
    lines.extend(["assert(API(code_can_unload)()==0); }", "static void unload_image(void) {",
                  "assert(API(code_can_unload)()==0); int closed=dlclose(image); assert(closed==0);"])
    lines.extend(f"{name}_slot=NULL;" for name in symbols)
    lines.extend(["image=NULL; }", f'#include "{source / "consumer/host.c"}"'])
    return "\n".join(lines) + "\n"
