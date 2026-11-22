/** @file
 * @brief Apply exact directory-relative or module-stem translation without textual import rewriting.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * fixes P(p) from explicit source/destination; [SRC-05](../../../../spec/language/source-and-names.md#src-05)
 * keeps plus storage in physical keys, never logical identities.
 */
#include <string.h>
#include "tree.h"
/** Directory suffixes keep component boundaries, including a filesystem-root source. */
static const char *directory_suffix(const char *path, const char *source) {
    size_t length = strlen(source);
    if (strncmp(path, source, length)) { return NULL; }
    const char *suffix = path + length;
    if (!*suffix) { return suffix; }
    if (source[length - 1] != '/' && *suffix != '/') { return NULL; }
    return *suffix == '/' ? suffix + 1 : suffix;
}
/** The module unit has three disjoint forms: main, dot-file companion, plus tree. */
static const char *module_suffix(const AcquisitionUnitEntry *entry, const char *source) {
    size_t length = strlen(source);
    if (strncmp(entry->file.path, source, length)) { return NULL; }
    const char *suffix = entry->file.path + length;
    if (!*suffix) { return entry->directory ? NULL : suffix; }
    if (*suffix == '.' && suffix[1]) { return entry->directory ? NULL : suffix; }
    if (*suffix == '+' && (suffix[1] == '/' || !suffix[1])) {
        return !suffix[1] && !entry->directory ? NULL : suffix;
    }
    return NULL;
}
SourcePlacementStatus source_tree_placement_build(SourceTreePlacement *placement,
    const SourceTreeSelection *selection, const char *destination) {
    if (!placement || placement->attempted || !selection || !selection->valid || !destination
        || destination[0] != '/' || !*destination) { return SOURCE_PLACEMENT_INVALID; }
    placement->attempted = true; placement->selection = selection;
    placement->domain_status = source_tree_selection_domain(selection, &placement->failed_path);
    if (placement->domain_status != SOURCE_TREE_DOMAIN_OK) { return placement->status = SOURCE_PLACEMENT_DOMAIN; }
    placement->destination = source_arena_copy(&placement->arena, destination);
    if (!placement->destination) { return placement->status = SOURCE_PLACEMENT_MEMORY; }
    bool directory = S_ISDIR(selection->operand.metadata.st_mode);
    if (!directory && (!destination[1] || destination[strlen(destination) - 1] == '/')) {
        return placement->status = SOURCE_PLACEMENT_INVALID;
    }
    SourceTreePlacementEntry **tail = &placement->entries;
    for (const AcquisitionUnitEntry *entry = selection->members; entry; entry = entry->next) {
        const char *suffix = directory ? directory_suffix(entry->file.path, selection->operand.path)
            : module_suffix(entry, selection->operand.path);
        if (!suffix) { placement->failed_path = entry->file.path; return placement->status = SOURCE_PLACEMENT_INVALID; }
        const char *separator = directory && *suffix && destination[strlen(destination) - 1] != '/' ? "/" : "";
        const char *path = source_arena_join(&placement->arena, placement->destination, separator, suffix);
        SourceTreePlacementEntry *row = path ? source_arena_allocate(&placement->arena, sizeof(*row)) : NULL;
        if (!row) { return placement->status = SOURCE_PLACEMENT_MEMORY; }
        *row = (SourceTreePlacementEntry){.original = entry, .path = path};
        *tail = row; tail = &row->next;
    }
    placement->ready = true;
    return placement->status = SOURCE_PLACEMENT_OK;
}
