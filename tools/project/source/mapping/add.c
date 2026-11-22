/** @file
 * @brief Admit clone proposals only when nominal and physical injectivity both hold.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * gives each original one identity and forbids content-based merging or implicit
 * duplicate specialization locations within an operation.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * preserves root kind and ordinary src/tests area before any row is published.
 */
#include <string.h>
#include "map.h"
#include "source/path/logical.h"
/** Reject foreign source handles before reading their original names or bytes. */
static bool owned(const SourceCloneMap *map, const ModuleSource *source) {
    for (const SourceCapturedUnit *unit = map->capture->units; unit; unit = unit->next) {
        if (unit->source == source) { return true; }
    }
    return false;
}
SourceCloneStatus source_clone_map_add(SourceCloneMap *map, const ModuleSource *source,
    const char *path, const char *identity, const SourceCloneMapping **output) {
    if (!map || map->ready) { return SOURCE_CLONE_INVALID; }
    if (map->status != SOURCE_CLONE_OK) { return map->status; }
    map->requester = NULL; map->conflict = NULL;
    if (!map->capture || !map->capture->valid || !output || !path || path[0] != '/' || !path[1]
        || path[strlen(path) - 1] == '/' || !identity
        || !module_path_canonical((const unsigned char *)identity, strlen(identity)) || !owned(map, source)) {
        return map->status = SOURCE_CLONE_INVALID;
    }
    map->requester = source;
    ModulePathShape original_shape, final_shape;
    if (!module_path_describe((const unsigned char *)source->logical_path,
        strlen(source->logical_path), &original_shape)
        || !module_path_describe((const unsigned char *)identity, strlen(identity), &final_shape)) {
        return map->status = SOURCE_CLONE_INVALID;
    }
    if (original_shape.root != final_shape.root || original_shape.area != final_shape.area) {
        return map->status = SOURCE_CLONE_NAMESPACE;
    }

    for (const SourceCloneMapping *entry = map->first; entry; entry = entry->next) {
        bool same_path = !strcmp(entry->path, path), same_name = !strcmp(entry->identity, identity);
        if (entry->original == source && same_path && same_name) { *output = entry; return SOURCE_CLONE_OK; }
        if (entry->original == source || same_path || same_name) {
            map->conflict = entry;
            return map->status = SOURCE_CLONE_CONFLICT;
        }
    }
    SourceCloneMapping *entry = source_arena_allocate(&map->arena, sizeof(*entry));
    const char *saved_path = entry ? source_arena_copy(&map->arena, path) : NULL;
    const char *saved_identity = saved_path ? source_arena_copy(&map->arena, identity) : NULL;
    if (!saved_identity) { return map->status = SOURCE_CLONE_MEMORY; }
    *entry = (SourceCloneMapping){.original = source, .path = saved_path, .identity = saved_identity};
    SourceCloneMapping **position = &map->first;
    while (*position && strcmp((*position)->original->logical_path, source->logical_path) < 0) { position = &(*position)->next; }
    entry->next = *position; *position = entry;
    *output = entry;
    return SOURCE_CLONE_OK;
}
