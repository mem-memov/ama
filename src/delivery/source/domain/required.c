/** @file
 * @brief Prepare the complete active captured domain for original-graph safety checks.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * includes every active definition, independent of executable reachability;
 * [SYN-04](../../../../spec/language/source-and-names.md#syn-04) retains drafts
 * in the catalog without activating their incomplete bodies.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains failed
 * preparation under caller-owned arenas and frontend lifetimes.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * binds captured companions only after native classification.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"
/** Byte order makes the required set independent of physical discovery order.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires exact original identities rather than prefix-based discovery guesses.
 */
static int compare(const void *left, const void *right) {
    return strcmp(*(const char *const *)left, *(const char *const *)right);
}
bool delivery_source_domain_required(CompilerFrontend *frontend, SourceArena *arena,
    const DeliverySourceCompanions *companions, const char *const **output, size_t *output_count) {
    size_t count = 0;
    for (const ModuleSource *source = frontend->project.catalog.first; source; source = source->next) {
        if (count == SIZE_MAX / sizeof(const char *)) { goto no_memory; }
        count++;
    }
    const char **names = count ? source_arena_allocate(arena, count * sizeof(*names)) : NULL;
    if (count && !names) { goto no_memory; }
    size_t used = 0;
    for (const ModuleSource *source = frontend->project.catalog.first; source; source = source->next) {
        AstModule syntax;
        ast_module_init(&syntax, arena->allocator);
        ParseStatus status = parser_module_parse(&syntax, source->text);
        if (status != PARSE_OK) { frontend->error = syntax.error.diagnostic; }
        else if (syntax.sections.kind != MODULE_DRAFT) { names[used++] = source->logical_path; }
        bool native = status == PARSE_OK && syntax.sections.kind == MODULE_NATIVE;
        ast_module_destroy(&syntax);
        if (native && companions && !delivery_source_domain_native(frontend, source, companions)) { return false; }
        if (status != PARSE_OK) { return false; }
    }
    if (used > 1) { qsort(names, used, sizeof(*names), compare); }
    *output = names; *output_count = used;
    return true;
no_memory:
    frontend->error = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY,
        .message = "Cannot retain the complete active source domain"};
    return false;
}
