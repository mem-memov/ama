/** @file
 * @brief Compose explicit library exports with actual typed projects and exact input snapshots.
 */
#ifndef AMA_LIBRARY_RESOLUTION_TEST_COMMON
#define AMA_LIBRARY_RESOLUTION_TEST_COMMON
#include "../common.c"
#include "source/identity/internal.h"
/** A complete wire declaration covers the actual public fields, never private state. */
static const char records[] = "schema | message | text | ..text\nschema | outcome | failure | ..failure\n"
    "schema | messages | list | message\nexport | run | .main | entry,resume,read\n"
    "export | other | .other | read\ninput | run | message text | message\noutput | run | failure | outcome\noutput | other | messages | messages\n";
/** Build two disconnected exported roots and deliberately invalid unexported source. */
static void prepare_project(void) {
    directory("generated/project");
    file("generated/project/main", "@<\nmessage text\n..text\n@>\nfailure\n..failure\n@!\n");
    file("generated/project/other", "@>\nmessages[]\n..text\n@!\n");
    file("generated/project/unselected", "this is not a valid executable source\n");
}
/** Parse then resolve one scenario against explicit project and standard root authorities. */
static bool resolve(LibraryBuild *build, const char *tail) {
    const SourceText *source = configuration(tail);
    library_build_init(build, sources.allocator);
    if (!library_configuration_parse(build, source)) { return false; }
    ModuleRoot roots[] = {{.path = "generated/project", .kind = MODULE_ROOT_PROJECT},
        {.path = TEST_STANDARD_ROOT, .kind = MODULE_ROOT_STANDARD}};
    return library_build_resolve(build, roots, sizeof(roots) / sizeof(*roots));
}

#endif
