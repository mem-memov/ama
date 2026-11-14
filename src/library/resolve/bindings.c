/** @file
 * @brief Prove exact, complete entry/result schema mappings against typed declarations.
 * [EMBED-04](../../../spec/implementation/c-library.md#embed-04) requires
 * unsupported ownership, arity and nominal mismatches to fail before publication.
 */
#include <string.h>
#include "../internal.h"
/** Compare the complete original interface name without trimming or suffix inference. */
static bool named(const SemanticSymbol *field, const char *name) {
    size_t length = field->name.end - field->name.start;
    return strlen(name) == length
        && memcmp(source_bytes(field->name.source) + field->name.start, name, length) == 0;
}
/** Resolve a single mapping by exact export/name/role, excluding private fields. */
static bool binding(LibraryBuild *build, LibraryBinding *item) {
    item->exported = library_export_find(build, item->export_name);
    item->schema = library_schema_find(build, item->schema_name);
    if (item->exported == NULL || item->schema == NULL
        || (item->input && (item->exported->operations & LIBRARY_ENTRY) == 0)) {
        return library_fail(build, item->site, "A transport mapping names an unavailable export, schema or entry operation");
    }
    SymbolRole role = item->input ? SYMBOL_PARAMETER : SYMBOL_RESULT;
    for (const SemanticSymbol *field = item->exported->type->first; field != NULL; field = field->next) {
        if (field->role == role && named(field, item->field_name)) { item->field = field; break; }
    }
    if (item->field == NULL || item->field->reference || item->field->type != item->schema->type
        || item->field->list != (item->schema->kind == LIBRARY_LIST)) {
        return library_fail(build, item->site, "The mapped interface field has no supported matching owned transport schema");
    }
    for (const LibraryBinding *earlier = build->bindings; earlier != item; earlier = earlier->next) {
        if (earlier->exported == item->exported && earlier->field == item->field) {
            return library_fail(build, item->site, "Duplicate library interface transport mapping");
        }
    }
    return true;
}
bool library_resolve_bindings(LibraryBuild *build) {
    for (LibraryBinding *item = build->bindings; item != NULL; item = item->next) {
        if (!binding(build, item)) { return false; }
    }
    for (const LibraryExport *exported = build->exports; exported != NULL; exported = exported->next) {
        for (const SemanticSymbol *field = exported->type->first; field != NULL; field = field->next) {
            if (field->role != SYMBOL_RESULT
                && !(field->role == SYMBOL_PARAMETER && (exported->operations & LIBRARY_ENTRY) != 0)) { continue; }
            bool found = false;
            for (const LibraryBinding *item = build->bindings; item != NULL; item = item->next) {
                if (item->exported == exported && item->field == field) { found = true; break; }
            }
            if (!found) {
                library_fail(build, exported->site, "Every exported interface field needs an explicit transport mapping");
                build->related = (DiagnosticRelated){{.span = field->site}, "unmapped exported field", exported->module};
                build->error.related = &build->related;
                build->error.related_count = 1;
                return false;
            }
        }
    }
    return true;
}
