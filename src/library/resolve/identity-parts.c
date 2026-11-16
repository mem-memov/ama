/** @file
 * @brief Frame the complete explicit ABI configuration for component-qualified names.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) requires
 * incompatible export/schema/settings contracts to have distinct checked keys.
 */
#include <stdint.h>
#include "../internal.h"
/** Prove a complete additional record fits both element and byte counts. Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06). */
static bool reserve(LibraryBuild *build, size_t *count, size_t addition) {
    if (*count > SIZE_MAX / sizeof(const char *) - addition) {
        return library_fail(build, (SourceSpan){0}, "Library identity exceeds the supported range");
    }
    *count += addition;
    return true;
}
bool library_identity_parts(LibraryBuild *build) {
    size_t count = 8;
    for (const LibrarySchema *s = build->schemas; s != NULL; s = s->next) {
        if (!reserve(build, &count, 4)) { return false; }
    }
    for (const LibraryExport *e = build->exports; e != NULL; e = e->next) {
        if (!reserve(build, &count, 4)) { return false; }
    }
    for (const LibraryBinding *b = build->bindings; b != NULL; b = b->next) {
        if (!reserve(build, &count, 5)) { return false; }
    }
    for (const LibrarySetting *s = build->settings; s != NULL; s = s->next) {
        if (!reserve(build, &count, 2)) { return false; }
    }
    const char **parts = library_allocate(build, count * sizeof(*parts));
    if (parts == NULL) { return false; }
    size_t index = 0;
    parts[index++] = "Ama C component";
    parts[index++] = build->component;
    parts[index++] = build->abi;
    parts[index++] = build->target;
    parts[index++] = build->calling_convention;
    static const char *const kinds[] = {"integer", "text", "condition", "failure", "list"};
    static const char *const modes[] = {"invalid", "entry", "resume", "entry,resume", "read", "entry,read", "resume,read", "entry,resume,read"};
    for (const LibrarySchema *s = build->schemas; s != NULL; s = s->next) {
        parts[index++] = "schema"; parts[index++] = s->name;
        parts[index++] = kinds[s->kind]; parts[index++] = s->argument;
    }
    for (const LibraryExport *e = build->exports; e != NULL; e = e->next) {
        parts[index++] = "export"; parts[index++] = e->name;
        parts[index++] = e->module; parts[index++] = modes[e->operations];
    }
    for (const LibraryBinding *b = build->bindings; b != NULL; b = b->next) {
        parts[index++] = b->input ? "input" : "output"; parts[index++] = b->export_name;
        parts[index++] = b->field_name; parts[index++] = b->schema_name;
        char ordinal[3 * sizeof(size_t) + 1];
        int length = snprintf(ordinal, sizeof(ordinal), "%zu", b->field->role_ordinal);
        if (length < 0 || (size_t)length >= sizeof(ordinal)) {
            return library_fail(build, b->site, "Library field ordinal exceeds its proven decimal bound");
        }
        char *copy = library_allocate(build, (size_t)length + 1);
        if (copy == NULL) { return false; }
        for (size_t i = 0; i <= (size_t)length; i++) { copy[i] = ordinal[i]; }
        parts[index++] = copy;
    }
    for (const LibrarySetting *s = build->settings; s != NULL; s = s->next) {
        parts[index++] = s->kind; parts[index++] = s->value;
    }
    build->identity_parts = parts;
    build->identity_part_count = index;
    return true;
}
