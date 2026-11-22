/** @file
 * @brief Preserve actual selected version and physical test provenance in neutral dependency views.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) forbids invented members;
 * [SRC-05](../../../../spec/language/source-and-names.md#src-05) distinguishes
 * physical plus storage from ordinary mirrored test directories.
 */
#include "catalog.h"
#include "internal.h"
#include <stdint.h>
#include <string.h>
/** Retain exact normalization diagnostics under
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) while translating
 * resource categories without inventing source origins for catalog names.
 */
static DependencySelectionStatus identity(SourceArena *arena, const char *text,
    const ModulePathValue **output, SourceIssue *issue) {
    ModulePathStatus status = module_path_from_canonical(arena, (const unsigned char *)text, strlen(text), output, issue);
    if (status == MODULE_PATH_OK) { return DEPENDENCY_SELECTION_OK; }
    if (status == MODULE_PATH_OUT_OF_MEMORY) { return DEPENDENCY_SELECTION_OUT_OF_MEMORY; }
    return status == MODULE_PATH_LIMIT ? DEPENDENCY_SELECTION_LIMIT : DEPENDENCY_SELECTION_INVALID;
}
/** Exact version/area bytes supply the only physical prefix stripped before the
 * common provenance validator. [PKG-08](../../../../spec/project/packages.md#pkg-08)
 * requires this physical evidence; logical ancestry cannot reconstruct '+' storage.
 */
static DependencySelectionStatus provenance(const ModuleSource *source,
    const ModulePathValue *path, DependencyViewInput *output, SourceIssue *issue) {
    ModulePathShape shape;
    (void)module_path_describe((const unsigned char *)path->text, path->length, &shape);
    *output = (DependencyViewInput){.identity = path};
    if (shape.area != MODULE_PATH_TESTS) { return DEPENDENCY_SELECTION_OK; }
    const char *physical = source_path(source->text);
    const char *version = source->version ? source->version->physical_path : NULL;
    size_t prefix = version ? strlen(version) : 0, length = strlen(physical);
    if (!version || length <= prefix || length - prefix <= 7 || memcmp(physical, version, prefix)
        || memcmp(physical + prefix, "/tests/", 7)) {
        return selection_failure(issue, (SourceSpan){source->text, 0, 0}, DEPENDENCY_SELECTION_INVALID,
            "Captured test module lacks exact selected-version physical provenance", "verified tests area-relative layout");
    }
    return dependency_view_provenance(path, (const unsigned char *)physical + prefix + 7,
        length - prefix - 7, output, issue);
}
/** Construct the complete actual ordinary universe required by
 * [PKG-05](../../../../spec/project/packages.md#pkg-05), retaining failed scratch
 * without publishing a partial view under [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
DependencySelectionStatus dependency_catalog_view(SourceArena *arena, const ModuleCatalog *catalog,
    const DependencyView **output, SourceIssue *issue) {
    if (!arena || !catalog || !catalog->built || !output || !issue) { return DEPENDENCY_SELECTION_INVALID; }
    size_t count = 0;
    for (const ModuleVersion *version = catalog->versions; version; version = version->next) {
        if (count == SIZE_MAX / sizeof(DependencyViewInput)) { goto limit; } count++;
    }
    for (const ModuleSource *source = catalog->first; source; source = source->next) {
        if (source->logical_path[1] == '/') { continue; }
        if (count == SIZE_MAX / sizeof(DependencyViewInput)) { goto limit; } count++;
    }
    DependencyViewInput *entries = count ? source_arena_allocate(arena, count * sizeof(*entries)) : NULL;
    if (count && !entries) {
        return selection_failure(issue, (SourceSpan){0}, DEPENDENCY_SELECTION_OUT_OF_MEMORY,
            "Catalog dependency view allocation failed", "available identity input storage");
    }
    size_t index = 0;
    for (const ModuleVersion *version = catalog->versions; version; version = version->next) {
        entries[index] = (DependencyViewInput){0};
        DependencySelectionStatus status = identity(arena, version->logical_path, &entries[index].identity, issue);
        if (status != DEPENDENCY_SELECTION_OK) { return status; } index++;
    }
    for (const ModuleSource *source = catalog->first; source; source = source->next) {
        if (source->logical_path[1] == '/') { continue; }
        const ModulePathValue *path = NULL;
        DependencySelectionStatus status = identity(arena, source->logical_path, &path, issue);
        if (status == DEPENDENCY_SELECTION_OK) { status = provenance(source, path, &entries[index], issue); }
        if (status != DEPENDENCY_SELECTION_OK) { return status; } index++;
    }
    return dependency_view_build(arena, entries, count, output, issue);
limit:
    return selection_failure(issue, (SourceSpan){0}, DEPENDENCY_SELECTION_LIMIT,
        "Catalog dependency view is too large", "a representable identity input array");
}
