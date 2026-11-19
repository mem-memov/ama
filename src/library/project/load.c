/** @file
 * @brief Compose explicit export closure loading, typed validation and component naming.
 * [EMBED-01](../../../spec/implementation/c-library.md#embed-01) separates
 * checked library configuration from executable generation and implicit roots.
 * [TPL-11](../../../spec/language/templates.md#tpl-11) registers the exact export/schema request as required Type roots.
 */
#include <assert.h>
#include <stdint.h>
#include "../internal.h"
#include "compiler/internal.h"
bool library_build_resolve(LibraryBuild *build, const ModuleRoot *roots, size_t count) {
    assert(!build->attempted);
    build->attempted = true;
    if (!build->parsed || build->error.message != NULL) { return false; }
    size_t required_count = build->export_count;
    for (const LibrarySchema *schema = build->schemas; schema != NULL; schema = schema->next) {
        if (schema->kind == LIBRARY_LIST) { continue; }
        if (required_count == SIZE_MAX) { return library_fail(build, schema->site, "Library schema count exceeds the supported range"); }
        required_count++;
        if (!module_path_valid(schema->argument)) { return library_fail(build, schema->site, "A scalar schema needs an exact logical module path"); }
    }
    if (required_count > SIZE_MAX / sizeof(const char *)) {
        return library_fail(build, (SourceSpan){0}, "Library export count exceeds the supported range");
    }
    const char **required = library_allocate(build, required_count * sizeof(*required));
    if (required == NULL) { return false; }
    size_t index = 0;
    for (const LibraryExport *exported = build->exports; exported != NULL; exported = exported->next) {
        required[index++] = exported->module;
    }
    for (const LibrarySchema *schema = build->schemas; schema != NULL; schema = schema->next) {
        if (schema->kind != LIBRARY_LIST) { required[index++] = schema->argument; }
    }
    /* Explicit extra schema modules allow an unused declared scalar schema to be
     * checked too. They are selected inputs, never catalog-wide inferred exports.
     */
    if (!compiler_frontend_load_required(&build->frontend, roots, count,
            required[0], required, required_count)) {
        build->error = build->frontend.error;
        return false;
    }
    if (!library_capture_inputs(build)) { return false; }
    if (!compiler_frontend_select_roots(&build->frontend, required, required_count, COMPILER_ROOTS_EXPLICIT)) {
        build->error = build->frontend.error;
        return false;
    }
    if (!compiler_frontend_finish(&build->frontend, NULL)) {
        build->error = build->frontend.error;
        return false;
    }
    if (!library_configuration_validate(build,&build->frontend.semantic)
        || !library_resolve_identities(build)) { return false; }
    build->valid = true;
    return true;
}
bool library_build_check(LibraryBuild *build, const ModuleRoot *roots, size_t count, const char *path) {
    assert(!build->attempted && build->configuration == NULL);
    const SourceText *source = NULL;
    SourceStatus status = source_manager_load(&build->sources, path, &source);
    if (status != SOURCE_OK) {
        build->attempted = true;
        build->error = (Diagnostic){.code = status == SOURCE_IO_ERROR ? DIAGNOSTIC_IO_FAILURE : DIAGNOSTIC_OUT_OF_MEMORY,
            .message = "Library configuration could not be read", .primary = {.path = path}};
        return false;
    }
    if (!library_configuration_parse(build, source)) { build->attempted = true; return false; }
    return library_build_resolve(build, roots, count);
}
