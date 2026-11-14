/** @file
 * @brief Own partial library-build state and allocation-free diagnostics.
 * [EMBED-01](../../spec/implementation/c-library.md#embed-01) requires failed
 * export validation to publish no library, without losing handwritten inputs.
 */
#include <string.h>
#include "internal.h"
void library_build_init(LibraryBuild *build, SourceAllocator allocator) {
    *build = (LibraryBuild){0};
    source_arena_init(&build->arena, allocator);
    source_manager_init(&build->sources, allocator);
    identity_registry_init(&build->identities, allocator);
    compiler_frontend_init(&build->frontend, allocator);
}
void library_build_destroy(LibraryBuild *build) {
    compiler_frontend_destroy(&build->frontend);
    identity_registry_destroy(&build->identities);
    source_arena_destroy(&build->arena);
    source_manager_destroy(&build->sources);
    *build = (LibraryBuild){0};
}
bool library_fail(LibraryBuild *build, SourceSpan site, const char *message) {
    build->valid = false;
    build->error = (Diagnostic){.code = DIAGNOSTIC_TYPE_MISMATCH,
        .message = message, .primary = {.span = site}};
    return false;
}
void *library_allocate(LibraryBuild *build, size_t size) {
    void *memory = source_arena_allocate(&build->arena, size);
    if (memory == NULL) {
        build->error = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY,
            .message = "Library configuration storage could not be allocated"};
    } else { memset(memory, 0, size); }
    return memory;
}
bool library_public_identifier(const char *name) {
    if (name == NULL || name[0] == '\0' || name[0] == '_') { return false; }
    for (size_t index = 0; name[index] != '\0'; index++) {
        unsigned char c = (unsigned char)name[index];
        bool letter = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        if (!letter && !(index != 0 && ((c >= '0' && c <= '9') || c == '_'))) { return false; }
    }
    return true;
}
LibrarySchema *library_schema_find(const LibraryBuild *build, const char *name) {
    for (LibrarySchema *schema = build->schemas; schema != NULL; schema = schema->next) {
        if (strcmp(schema->name, name) == 0) { return schema; }
    }
    return NULL;
}
LibraryExport *library_export_find(const LibraryBuild *build, const char *name) {
    for (LibraryExport *exported = build->exports; exported != NULL; exported = exported->next) {
        if (strcmp(exported->name, name) == 0) { return exported; }
    }
    return NULL;
}
