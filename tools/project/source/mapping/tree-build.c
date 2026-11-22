/** @file
 * @brief Apply exact directory-relative or module-stem translation without textual import rewriting.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * fixes P(p) from explicit source/destination; [SRC-05](../../../../spec/language/source-and-names.md#src-05)
 * keeps plus storage in physical keys, never logical identities.
 */
#include <string.h>
#include "tree.h"
#include "unit/member.h"
/** Directory suffixes keep component boundaries, including a filesystem-root source. */
static const char *directory_suffix(const char *path, const char *source) {
    size_t length = strlen(source);
    if (strncmp(path, source, length)) { return NULL; }
    const char *suffix = path + length;
    if (!*suffix) { return suffix; }
    if (source[length - 1] != '/' && *suffix != '/') { return NULL; }
    return *suffix == '/' ? suffix + 1 : suffix;
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
        const char *path = NULL;
        if (directory) {
            const char *suffix = directory_suffix(entry->file.path, selection->operand.path);
            if (!suffix) { placement->failed_path = entry->file.path; return placement->status = SOURCE_PLACEMENT_INVALID; }
            const char *separator = *suffix && destination[strlen(destination) - 1] != '/' ? "/" : "";
            path = source_arena_join(&placement->arena, placement->destination, separator, suffix);
        } else {
            SourceUnitPathStatus status = source_unit_member_path(&placement->arena, selection->operand.path,
                placement->destination, entry->file.path, entry->directory, &path);
            if (status != SOURCE_UNIT_PATH_OK) {
                placement->failed_path = entry->file.path;
                return placement->status = status == SOURCE_UNIT_PATH_MEMORY ? SOURCE_PLACEMENT_MEMORY : SOURCE_PLACEMENT_INVALID;
            }
        }
        SourceTreePlacementEntry *row = path ? source_arena_allocate(&placement->arena, sizeof(*row)) : NULL;
        if (!row) { return placement->status = SOURCE_PLACEMENT_MEMORY; }
        *row = (SourceTreePlacementEntry){.original = entry, .path = path};
        *tail = row; tail = &row->next;
    }
    placement->ready = true;
    return placement->status = SOURCE_PLACEMENT_OK;
}
