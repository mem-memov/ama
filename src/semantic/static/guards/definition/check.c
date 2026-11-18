/** @file
 * @brief Reject directed cycles in the scoped static-definition graph after exact owner-key merging.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates definition and equality cycles.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) releases successful scratch.
 * [COMP-04](../../../../../spec/implementation/compiler.md#comp-04) retains cycle witnesses.
 */
#include <stdint.h>
#include <string.h>
#include "../internal.h"

/** Existing search storage becomes the failed owner's evidence; no allocation
 * is needed after observing the cycle. Original scoped nodes remain retained.
 * [COMP-04](../../../../../spec/implementation/compiler.md#comp-04) preserves source chains.
 */
static bool reject(StaticGuardPlan *plan, StaticDefinitionSearch *current,
    const StaticDefinitionEdge *closing, SourceArena *scratch) {
    (void)static_guard_fail(plan, STATIC_GUARD_CONFLICT, closing->site,
        "Cyclic cached static value-definition dependency");
    plan->definition_witness = current;
    plan->definition_closing_edge = closing;
    plan->rejected_definition = closing->target;
    plan->rejected_site = closing->site;
    plan->witness_arena = *scratch;
    source_arena_init(scratch, plan->arena.allocator);
    const StaticSymbol *symbol = closing->target->symbol;
    plan->related[0] = (DiagnosticRelated){.location = {.span = symbol->site},
        .role = "cyclic static definition", .name = symbol->owner->definition->source->logical_path};
    plan->related[1] = (DiagnosticRelated){.location = {.span = closing->site},
        .role = "closing definition dependency", .name = current->node->symbol->owner->definition->source->logical_path};
    plan->error.related = plan->related;
    plan->error.related_count = 2;
    return false;
}

/** Edges stay on their original scoped nodes; enumerate each representative's
 * complete edge union without replacing source identity or guessing aliases.
 * A nonempty return path to start is exactly a directed definition cycle.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves merged edges.
 */
static bool search(StaticGuardPlan *plan, StaticDefinitionNode *start,
    StaticDefinitionSearch *entries, StaticDefinitionSearch **seen, SourceArena *scratch) {
    memset(seen, 0, plan->definition_count * sizeof(*seen));
    entries[0] = (StaticDefinitionSearch){.node = start};
    seen[start->ordinal] = &entries[0];
    size_t count = 1;
    for (size_t front = 0; front < count; front++) {
        StaticDefinitionSearch *current = &entries[front];
        for (StaticDefinitionNode *source = plan->first_definition; source; source = source->next) {
            if (source->representative != current->node) { continue; }
            for (const StaticDefinitionEdge *edge = source->first; edge; edge = edge->next) {
                StaticDefinitionNode *target = edge->target->representative;
                if (target == start) { return reject(plan, current, edge, scratch); }
                if (seen[target->ordinal] != NULL) { continue; }
                entries[count] = (StaticDefinitionSearch){.node = target, .parent = current, .edge = edge};
                seen[target->ordinal] = &entries[count++];
            }
        }
    }
    return true;
}

/** A graph is acyclic iff no representative has a nonempty path to itself.
 * Search finite scoped representatives iteratively; equality/reference relations
 * are absent from this relation and therefore cannot create false cycles.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) rejects definition cycles.
 */
bool static_guard_definition_graph(StaticGuardPlan *plan) {
    if (plan->definition_edge_count == 0) { return true; }
    if (plan->definition_count > SIZE_MAX / sizeof(StaticDefinitionSearch) ||
        plan->definition_count > SIZE_MAX / sizeof(StaticDefinitionSearch *)) {
        return static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, (SourceSpan){0},
            "Static definition search exceeds representable storage");
    }
    SourceArena scratch;
    source_arena_init(&scratch, plan->arena.allocator);
    StaticDefinitionSearch *entries = source_arena_allocate(&scratch, plan->definition_count * sizeof(*entries));
    StaticDefinitionSearch **seen = entries ? source_arena_allocate(&scratch, plan->definition_count * sizeof(*seen)) : NULL;
    bool success = entries != NULL && seen != NULL;
    if (!success) {
        (void)static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, (SourceSpan){0},
            "Static definition search could not allocate scratch");
    }
    for (StaticDefinitionNode *node = plan->first_definition; success && node; node = node->next) {
        if (node->representative != node) { continue; }
        success = search(plan, node, entries, seen, &scratch);
    }
    source_arena_destroy(&scratch);
    return success;
}
