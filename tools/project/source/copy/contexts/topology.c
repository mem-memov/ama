/** @file
 * @brief Resolve captured context provenance through exact identities and physical plus ancestry.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * requires final structural providers rather than an original basename guess.
 */
#include "internal.h"
#include <string.h>
/** Exact final names are already canonical and unique in checked captures. */
const SourceCapturedUnit *source_copy_context_unit(const SourceCapturedUnit *units, const char *identity) {
    while (units && strcmp(units->source->logical_path, identity)) { units = units->next; }
    return units;
}
/** Original and final main queries use the same injective relation. */
const SourceCloneMapping *source_copy_context_clone(const SourceCloneMap *map, const char *identity, bool original) {
    const SourceCloneMapping *row = map->first;
    while (row && strcmp(original ? row->original->logical_path : row->identity, identity)) { row = row->next; }
    return row;
}
/** Both logical and physical boundaries are required, keeping nested configured
 * roots from turning an unrelated physical prefix into a module ancestor. */
bool source_copy_context_parent(const ModuleSource *parent, const ModuleSource *child) {
    size_t name_length = strlen(parent->logical_path), path_length = strlen(source_path(parent->text));
    const char *path = source_path(child->text);
    return !strncmp(child->logical_path, parent->logical_path, name_length)
        && child->logical_path[name_length] == '/'
        && !strncmp(path, source_path(parent->text), path_length)
        && path[path_length] == '+' && path[path_length + 1] == '/';
}
