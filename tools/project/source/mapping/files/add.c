/** @file
 * @brief Join overlapping structural maps by exact original keys, shapes and bytes.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * rejects different destinations for one input and distinct inputs at one output.
 * Prepare all allocations before publishing a new row.
 */
#include <string.h>
#include "internal.h"
/** Paths are already normalized; enforce only the prepared-key interface shape. */
static bool key(const char *path) {
    return path && path[0] == '/' && (path[1] == 0 || path[strlen(path) - 1] != '/');
}
/** Use one key/payload contract for borrowed captures and owned closure adapters. */
bool source_file_map_entry_valid(const AcquisitionUnitEntry *entry, const char *destination) {
    return entry && key(entry->file.path) && key(destination) && entry->file.state == DELIVERY_FILE_PRESENT
        && (entry->directory || (entry->file.path[1] && destination[1]))
        && (entry->directory ? entry->file.content == NULL : entry->file.content != NULL);
}
/** Independent captures of one original path must agree in shape and every byte. */
static bool same(const AcquisitionUnitEntry *left, const AcquisitionUnitEntry *right) {
    return left->directory == right->directory && (left->directory
        || (source_length(left->file.content) == source_length(right->file.content)
            && !memcmp(source_bytes(left->file.content), source_bytes(right->file.content), source_length(left->file.content))));
}
SourceFileMapStatus source_file_map_add(SourceFileMap *map, const AcquisitionUnitEntry *entry,
    const char *destination, const SourceFileMapping **output) {
    if (!map || map->ready) { return SOURCE_FILE_MAP_INVALID; }
    if (map->status != SOURCE_FILE_MAP_OK) { return map->status; }
    if (!output || !source_file_map_entry_valid(entry, destination)) {
        return map->status = SOURCE_FILE_MAP_INVALID;
    }
    map->requester = entry;
    for (const SourceFileMapping *row = map->first; row; row = row->next) {
        bool original = !strcmp(row->source, entry->file.path);
        bool target = !strcmp(row->destination, destination);
        if (original && target && same(row->original, entry)) { *output = row; return SOURCE_FILE_MAP_OK; }
        if (original || target) { map->conflict = row; return map->status = SOURCE_FILE_MAP_CONFLICT; }
    }
    SourceFileMapping *row = source_arena_allocate(&map->arena, sizeof(*row));
    const char *source = row ? source_arena_copy(&map->arena, entry->file.path) : NULL;
    const char *target = source ? source_arena_copy(&map->arena, destination) : NULL;
    if (!target) { return map->status = SOURCE_FILE_MAP_MEMORY; }
    SourceFileMapping **position = &map->first;
    while (*position && strcmp((*position)->source, source) < 0) { position = &(*position)->next; }
    *row = (SourceFileMapping){.next = *position, .original = entry, .source = source, .destination = target};
    *position = row; *output = row;
    return SOURCE_FILE_MAP_OK;
}
SourceFileMapStatus source_file_map_tree(SourceFileMap *map, const SourceTreePlacement *placement) {
    if (!map || map->ready) { return SOURCE_FILE_MAP_INVALID; }
    if (map->status != SOURCE_FILE_MAP_OK) { return map->status; }
    if (!placement || !placement->ready) { return map->status = SOURCE_FILE_MAP_INVALID; }
    for (const SourceTreePlacementEntry *row = placement->entries; row; row = row->next) {
        const SourceFileMapping *output = NULL;
        SourceFileMapStatus status = source_file_map_add(map, row->original, row->path, &output);
        if (status != SOURCE_FILE_MAP_OK) { return status; }
    }
    return SOURCE_FILE_MAP_OK;
}
