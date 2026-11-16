/** @file
 * @brief Derive the complete public definition set from checked export declarations.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) requires
 * private runtime/native definitions to remain local even when two components
 * contain identical logical modules. An exact allowlist avoids treating a
 * spelling prefix as proof that an arbitrary native symbol is public.
 */
#include <assert.h>
#include "library/model.h"
/** Append a declared implementation name; preserve stream failure for the caller. Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06). */
static bool symbol(FILE *output, const char *prefix, const char *operation) {
    return fprintf(output, "%s_%s_implementation\n", prefix, operation) >= 0;
}
bool library_symbols_write(const LibraryBuild *build, FILE *output) {
    assert(build->valid);
    if (!library_header_target(build, NULL)) { return false; }
    static const char *const lifecycle[] = {"abi", "context_create", "context_dispose",
        "instance_dispose", "diagnostic", "result_release", "diagnostic_release",
        "code_pin", "code_unpin", "code_can_unload"};
    for (size_t index = 0; index < sizeof(lifecycle) / sizeof(*lifecycle); index++) {
        if (!symbol(output, build->identity->internal_name, lifecycle[index])) { return false; }
    }
    for (const LibrarySchema *schema = build->schemas; schema != NULL; schema = schema->next) {
        if (!symbol(output, schema->identity->internal_name, "view")
            || !symbol(output, schema->identity->internal_name, "clone")
            || !symbol(output, schema->identity->internal_name, "release")) { return false; }
    }
    for (const LibraryExport *exported = build->exports; exported != NULL; exported = exported->next) {
        const char *prefix = exported->identity->internal_name;
        if (!symbol(output, prefix, "create")) { return false; }
        if ((exported->operations & LIBRARY_ENTRY) != 0 && !symbol(output, prefix, "entry")) { return false; }
        if ((exported->operations & LIBRARY_RESUME) != 0 && !symbol(output, prefix, "resume")) { return false; }
        if ((exported->operations & LIBRARY_READ) != 0 && !symbol(output, prefix, "read")) { return false; }
    }
    return fflush(output) == 0;
}
