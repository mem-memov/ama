/** @file
 * @brief Supply minimal linked project hints independently of the compiler edition's dispatcher.
 * [PROJECT-02](../../../spec/implementation/project-tools.md#project-02)
 * keeps vendor, package, version and entry choices with the programmer.
 */
#include "content.h"
static const ProjectInitFile files[] = {
    {"README.md",
        "# Ama project\n\n"
        "Use this project to develop or compose independently tested Ama modules.\n"
        "See [agent guidance](AGENTS.md) and [package layout](packages/README.md).\n\n"
        "`packages/` holds maintained code; optional `dependencies/` holds acquired code.\n"
        "Either root may be absent when the other supplies the selected modules.\n"
        "A maintained version in packages/ overrides that entire dependency version.\n"
        "Add your first vendor/package/version as shown in the package guide, or\n"
        "explicitly acquire dependencies with repository mappings; compilation never downloads code.\n"
        "Temporary download/ exists only when acquisition needs staging.\n\n"
        "Select any ordinary executable module explicitly, for example:\n\n"
        "```sh\nama --emit . /vendor/package/v1.0.0/src/module --out OUTPUT --sdk SDK --standard-root STANDARD --mode executable --component COMPONENT --compile\n```\n\n"
        "This is a command shape: choose real names, tool installation paths and an output destination.\n"
        "Initialization creates no entry module or package.ini.\n"},
    {"AGENTS.md",
        "# Guidance for Ama contributors\n\n"
        "Read the [project overview](README.md) and [layout](packages/README.md).\n"
        "Navigate by full /vendor/package/version/src-or-tests/module identity.\n"
        "Use exactly one ASCII space for each Ama indentation level; never use tabs.\n"
        "Keep a module's helpers in module+/ and its .ini, .c, .h, .abi and .cmake\n"
        "companions together with its main Ama file.\n"
        "Use package.ini for dependencies and root aliases; module.ini can add\n"
        "local ~aliases and use package aliases. Static names begin with one dot\n"
        "and contain no slashes; they are not filesystem paths.\n"
        "Definitions in src must not explicitly depend on tests, including static\n"
        "constructions; tests may supply their own types to generic parameters.\n"
        "Retain relevant independent test microprograms and run them explicitly.\n"
        "Use explicit ama-project operations for source mutations; compilation\n"
        "must not download, rewrite sources or execute tests implicitly.\n"},
    {"packages/README.md",
        "# Versioned packages\n\n"
        "Return to the [overview](../README.md) or [agent guidance](../AGENTS.md).\n\n"
        "An example to create later (not generated automatically):\n\n"
        "```text\nvendor/package/v1.0.0/\n  package.ini\n  src/\n    module\n  tests/\n    module/\n      empty input\n```\n\n"
        "package.ini lists selected dependencies, repository mappings and aliases;\n"
        "vendor/package/version identity already comes from the directory names.\n"
        "Tests mirror source ownership and are ordinary independent programs.\n"
        "Their helper modules stay in tests. Keep code and the tests needed to\n"
        "repeat its experiment together. Any ordinary module can be selected as\n"
        "an entry; there is no mandatory main module. A project composed entirely\n"
        "of acquired code may omit packages/ and use dependencies/ instead.\n"}
};
const ProjectInitFile *project_init_content(size_t *count) {
    *count = sizeof(files) / sizeof(files[0]);
    return files;
}
