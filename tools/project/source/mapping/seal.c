/** @file
 * @brief Publish only a complete unfailed injective clone relation.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids using a conflicted/partial proposal set as a destination map.
 */
#include "map.h"
SourceCloneStatus source_clone_map_seal(SourceCloneMap *map) {
    if (!map || map->ready) { return SOURCE_CLONE_INVALID; }
    if (map->status != SOURCE_CLONE_OK) { return map->status; }
    if (!map->capture || !map->capture->valid) { return map->status = SOURCE_CLONE_INVALID; }
    map->ready = true;
    return SOURCE_CLONE_OK;
}
