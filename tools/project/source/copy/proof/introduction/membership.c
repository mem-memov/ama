/** @file
 * @brief Query exact introduced and copied identities without conflating nominal names with origins.
 * [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * distinguishes independent suppliers and explicit C substitutions.
 */
#include "internal.h"
#include <string.h>
bool source_copy_introduction_contains(const SourceCopyIntroduction *introduced, const char *name) {
    for (size_t i = 0; i < introduced->count; i++) {
        if (!strcmp(introduced->units[i]->source->logical_path, name)) { return true; }
    }
    return false;
}
const SourceCloneMapping *source_copy_introduction_clone(const SourceCloneMap *map, const char *name, bool original) {
    for (const SourceCloneMapping *row = map->first; row; row = row->next) {
        if (!strcmp(original ? row->original->logical_path : row->identity, name)) { return row; }
    }
    return NULL;
}
