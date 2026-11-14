/** @file
 * @brief Name the component, schemas and exports through full collision-checked keys.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) binds public
 * names to component identity, complete export identity and the declared C ABI.
 */
#include <stdint.h>
#include "../internal.h"
/** Register the exact framed tuple, preserving unequal-key collision as a hard failure. */
static bool identity(LibraryBuild *build, const char *module, const char *role,
    const char *name, const char *detail, SourceSpan site, const IdentityName **output) {
    size_t count = build->identity_part_count;
    build->identity_parts[count] = role;
    build->identity_parts[count + 1] = name;
    build->identity_parts[count + 2] = detail;
    IdentityStatus status = identity_register(&build->identities,
        (QualifiedIdentity){IDENTITY_RUNTIME, module, build->identity_parts, count + 3, site}, output);
    if (status == IDENTITY_OK) { return true; }
    library_fail(build, site, status == IDENTITY_COLLISION
        ? "Distinct library identities produce the same public symbol"
        : "A complete library identity could not be registered");
    if (status == IDENTITY_OUT_OF_MEMORY) { build->error.code = DIAGNOSTIC_OUT_OF_MEMORY; }
    if (status == IDENTITY_COLLISION) {
        build->error.code = DIAGNOSTIC_IDENTITY_COLLISION;
        build->error.subject = name;
        build->related = (DiagnosticRelated){{.span = build->identities.conflicting->declaration},
            "conflicting complete library identity", build->identities.conflicting->source_name};
        build->error.related = &build->related;
        build->error.related_count = 1;
    }
    return false;
}
bool library_resolve_identities(LibraryBuild *build) {
    if (!library_identity_parts(build)) { return false; }
    SourceSpan site = {build->configuration, 0, source_length(build->configuration)};
    if (!identity(build, ".Ama C component", "component", build->component, build->abi, site, &build->identity)) { return false; }
    for (LibrarySchema *schema = build->schemas; schema != NULL; schema = schema->next) {
        const char *role = schema->kind == LIBRARY_LIST ? "owning list schema" : "owned scalar schema";
        const char *module = schema->type->module->source->logical_path;
        if (!identity(build, module, role, schema->name, schema->argument, schema->site, &schema->identity)) { return false; }
    }
    for (LibraryExport *exported = build->exports; exported != NULL; exported = exported->next) {
        static const char *const modes[] = {"invalid", "entry", "resume", "entry,resume", "read", "entry,read", "resume,read", "entry,resume,read"};
        if (!identity(build, exported->module, "export", exported->name, modes[exported->operations], exported->site, &exported->identity)) { return false; }
    }
    return true;
}
