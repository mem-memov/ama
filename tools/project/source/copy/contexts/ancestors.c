/** @file
 * @brief Derive exact ancestor positions from captured mains rather than path-component counts.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * requires matching nearest-first positions; [SRC-05](../../../../../spec/language/source-and-names.md#src-05)
 * excludes ordinary directories from the module ancestor chain. Both planning
 * queries and final proofs use this same finite structural operation.
 */
#include "internal.h"
bool source_copy_context_position(const SourceCapturedUnit *units, const ModuleSource *parent,
    const ModuleSource *child, size_t *output) {
    if (!output || !parent || !child) { return false; }
    bool found_parent = false, found_child = false;
    /* Membership precedes every name/text read: a foreign handle need not have
     * initialized module fields and grants no authority to inspect them. */
    for (const SourceCapturedUnit *unit = units; unit; unit = unit->next) {
        found_parent |= unit->source == parent; found_child |= unit->source == child;
    }
    if (!found_parent || !found_child || !source_copy_context_parent(parent, child)) { return false; }
    size_t position = 0;
    for (const SourceCapturedUnit *unit = units; unit; unit = unit->next) {
        if (source_copy_context_parent(parent, unit->source)
            && source_copy_context_parent(unit->source, child)) { position++; }
    }
    *output = position; return true;
}
const SourceCapturedUnit *source_copy_context_ancestor(const SourceCapturedUnit *units,
    const ModuleSource *child, size_t position) {
    const SourceCapturedUnit *found = NULL;
    for (const SourceCapturedUnit *unit = units; unit; unit = unit->next) {
        size_t actual;
        if (!source_copy_context_position(units, unit->source, child, &actual) || actual != position) { continue; }
        if (found) { return NULL; } found = unit;
    }
    return found;
}
