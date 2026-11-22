/** @file
 * @brief Preserve each known original input use with its original owner and captured-file witness.
 * [TOOL-03](../../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * reports each surviving requirement; no diagnostic string or C-expression scan
 * can substitute for typed captured provenance.
 */
#include "internal.h"
SourceRemovalSelectionStatus source_removal_input_append(SourceRemovalInputBuilder *builder,
    const DeliverySourceDefinition *owner, const char *path, const SourceText *origin) {
    if (!path || !*path) { return SOURCE_REMOVAL_SELECTION_INVALID; }
    SourceRemovalInputUse *input = source_arena_allocate(&builder->selection->arena, sizeof(*input));
    if (!input) { return SOURCE_REMOVAL_SELECTION_MEMORY; }
    *input = (SourceRemovalInputUse){.owner = owner, .path = path, .origin = {origin, 0, 0}};
    *builder->tail = input; builder->tail = &input->next;
    return SOURCE_REMOVAL_SELECTION_OK;
}
