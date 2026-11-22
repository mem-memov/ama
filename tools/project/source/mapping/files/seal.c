/** @file
 * @brief Refuse final file-as-directory topology before freezing a physical relation.
 * [SRC-05](../../../../../spec/language/source-and-names.md#src-05)
 * requires real directory storage; [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids accepting inconsistent overlapping maps, even before publication.
 */
#include <string.h>
#include "files.h"
/** Exact component ancestry distinguishes /a/b from /a/bc and handles root /. */
static bool below(const char *path, const char *parent) {
    size_t length = strlen(parent);
    return !strncmp(path, parent, length) && path[length]
        && (parent[length - 1] == '/' || path[length] == '/');
}
SourceFileMapStatus source_file_map_seal(SourceFileMap *map) {
    if (!map || map->ready) { return SOURCE_FILE_MAP_INVALID; }
    if (map->status != SOURCE_FILE_MAP_OK) { return map->status; }
    for (const SourceFileMapping *parent = map->first; parent; parent = parent->next) {
        if (parent->original->directory) { continue; }
        for (const SourceFileMapping *child = map->first; child; child = child->next) {
            if (below(child->destination, parent->destination)) {
                map->conflict = parent; map->related = child;
                return map->status = SOURCE_FILE_MAP_CONFLICT;
            }
        }
    }
    map->ready = true;
    return SOURCE_FILE_MAP_OK;
}
