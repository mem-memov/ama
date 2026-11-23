/** @file
 * @brief Restrict only final author conservation while preserving every original incoming target.
 * [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids an unchanged nominal name from hiding replacement of an original target.
 */
#include "internal.h"
#include <string.h>
/** Exact original author identity is distinct from the final copied identity. */
static const DeliverySourceDefinition *definition(const DeliverySourceDefinition *definitions, const char *identity) {
    while (definitions && strcmp(definitions->identity, identity)) { definitions = definitions->next; }
    return definitions;
}
SourceIntroductionStatus source_copy_introduction_graph(SourceArena *arena, const SourceCopyIntroduction *introduced,
    const SourceCloneMap *map, const DeliverySourceDefinition *original, const DeliverySourceDefinition *final,
    const DeliverySourceDefinition **output, SourceIntroductionIssue *issue) {
    if (!introduced->count) { *output = final; return SOURCE_INTRODUCTION_OK; }
    DeliverySourceDefinition *first = NULL, *last = NULL;
    for (const DeliverySourceDefinition *now = final; now; now = now->next) {
        if (source_copy_introduction_contains(introduced, now->identity)) { continue; }
        const SourceCloneMapping *copy = source_copy_introduction_clone(map, now->identity, false);
        const DeliverySourceDefinition *old = definition(original, copy ? copy->original->logical_path : now->identity);
        for (const DeliverySourceDependency *edge = old ? old->dependencies : NULL; edge; edge = edge->next) {
            if (source_copy_introduction_contains(introduced, edge->target)
                && !(copy && source_copy_introduction_clone(map, edge->target, true))) {
                issue->edge = edge; issue->path = edge->target; return SOURCE_INTRODUCTION_TARGET;
            }
        }
        DeliverySourceDefinition *row = source_arena_allocate(arena, sizeof(*row));
        if (!row) { return SOURCE_INTRODUCTION_MEMORY; }
        *row = *now; row->next = NULL;
        if (last) { last->next = row; } else { first = row; } last = row;
    }
    *output = first; return SOURCE_INTRODUCTION_OK;
}
