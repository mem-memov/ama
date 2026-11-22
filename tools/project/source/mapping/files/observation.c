/** @file
 * @brief Own durable captured-entry adapters for immutable closure membership facts.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * joins overlapping captures by exact kind/bytes; [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * keeps incoming conflict witnesses alive through failed-map teardown.
 */
#include "internal.h"
SourceFileMapStatus source_file_map_observation(SourceFileMap *map, const char *source,
    const SourceText *content, bool directory, const char *destination, const SourceFileMapping **output) {
    if (!map || map->ready) { return SOURCE_FILE_MAP_INVALID; }
    if (map->status != SOURCE_FILE_MAP_OK) { return map->status; }
    AcquisitionUnitEntry candidate = {.file = {.path = source, .content = content,
        .state = DELIVERY_FILE_PRESENT, .issue = PATH_FOUND}, .directory = directory};
    if (!output || !source_file_map_entry_valid(&candidate, destination)) { return map->status = SOURCE_FILE_MAP_INVALID; }
    AcquisitionUnitEntry *owned = source_arena_allocate(&map->arena, sizeof(*owned));
    if (!owned) { return map->status = SOURCE_FILE_MAP_MEMORY; }
    *owned = candidate;
    return source_file_map_add(map, owned, destination, output);
}
