/** @file
 * @brief Join physical tree membership to original module handles without sibling activation.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * makes a bare plus-directory independent of its sibling main;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * rejects changed original observations before exposing selection metadata.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"
#include "../capture/match.h"
bool source_tree_inside(const char *path, const char *parent) {
    size_t length = strlen(parent);
    return !strncmp(path, parent, length) && (!path[length] || parent[length - 1] == '/' || path[length] == '/');
}
/** Match original physical storage, not a substituted logical name prefix. */
static bool covered(const SourceTreeSelection *selection, const SourceCapturedUnit *unit) {
    const char *path = source_path(unit->source->text), *root = selection->operand.path;
    if (S_ISDIR(selection->operand.metadata.st_mode)) { return source_tree_inside(path, root); }
    size_t length = strlen(root);
    return !strcmp(path, root) || (!strncmp(path, root, length) && path[length] == '+' && path[length + 1] == '/');
}
SourceTreeStatus source_tree_project(SourceTreeSelection *selection, const SourceRootCapture *capture) {
    size_t count = 0;
    for (const SourceCapturedUnit *unit = capture->units; unit; unit = unit->next) {
        if (!covered(selection, unit)) { continue; }
        if (!source_captured_unit_matches(unit, selection->members)) {
            selection->failed_path = source_path(unit->source->text);
            return SOURCE_TREE_CHANGED;
        }
        if (count == SIZE_MAX) { return SOURCE_TREE_MEMORY; }
        count++;
    }
    if (count > SIZE_MAX / sizeof(const SourceCapturedUnit *)) { return SOURCE_TREE_MEMORY; }
    const SourceCapturedUnit **units = count ? source_arena_allocate(&selection->arena, count * sizeof(*units)) : NULL;
    if (count && !units) { return SOURCE_TREE_MEMORY; }
    size_t index = 0;
    for (const SourceCapturedUnit *unit = capture->units; unit; unit = unit->next) {
        if (covered(selection, unit)) { units[index++] = unit; }
    }
    selection->units = units;
    selection->count = count;
    return SOURCE_TREE_OK;
}
