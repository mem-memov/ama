/** @file
 * @brief Review exact original removal material against all authored and explicit client roots.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * requires both cuts before replacement or source removal; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * still requires later final graph/topology/freshness and recoverable publication.
 */
#include "selection.h"
SourceRemovalStatus source_removal_selection_review(SourceArena *arena, const SourceRemovalSelection *selection,
    const SourceRemovalInputUse *inputs, const SourceClientRoot *roots, SourceRemovalReview *output) {
    if (!arena || !selection || !selection->valid || !output) { return SOURCE_REMOVAL_INVALID; }
    SourceRemovalInputUse *combined = NULL, **tail = &combined;
    for (const SourceRemovalInputUse *input = inputs; input; input = input->next) {
        SourceRemovalInputUse *copy = source_arena_allocate(arena, sizeof(*copy));
        if (!copy) { return SOURCE_REMOVAL_OUT_OF_MEMORY; }
        *copy = *input; copy->next = NULL;
        *tail = copy; tail = &copy->next;
    }
    *tail = selection->inputs;
    SourceRemovalReview result = {0};
    const DeliverySourceDefinition *definitions = selection->analysis->original.response.definitions;
    SourceRemovalStatus consumers = source_removal_check(arena, definitions, combined,
        selection->identities, selection->identity_count, selection->paths, selection->path_count, &result.consumers);
    if (consumers != SOURCE_REMOVAL_OK && consumers != SOURCE_REMOVAL_BLOCKED) { return consumers; }
    SourceRemovalStatus clients = source_removal_client_roots(arena, definitions, roots,
        selection->identities, selection->identity_count, &result.clients);
    if (clients != SOURCE_REMOVAL_OK && clients != SOURCE_REMOVAL_BLOCKED) { return clients; }
    *output = result;
    return consumers == SOURCE_REMOVAL_BLOCKED || clients == SOURCE_REMOVAL_BLOCKED ? SOURCE_REMOVAL_BLOCKED : SOURCE_REMOVAL_OK;
}
