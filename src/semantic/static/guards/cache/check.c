/** @file
 * @brief Reject nonempty concrete paths with repeated original template labels, including cached descendants.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) checks full-key creation paths.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) releases successful search scratch.
 * [COMP-04](../../../../../spec/implementation/compiler.md#comp-04) retains complete rejected witnesses.
 */
#include <stdint.h>
#include <string.h>
#include "../internal.h"

/** Move already allocated search evidence into the failed owner rather than
 * allocating to report a semantic error. All witness entries and original
 * source edges remain available to the later diagnostic-chain renderer.
 * [COMP-04](../../../../../spec/implementation/compiler.md#comp-04) preserves full context.
 */
static bool reject(StaticGuardPlan *plan, StaticCreationSearch *current,
    StaticCreationEdge *closing, SourceArena *scratch) {
    const StaticModule *origin = closing->target->value->as.nominal.origin;
    SourceSpan site = closing->path->site;
    (void)static_guard_fail(plan, STATIC_GUARD_CONFLICT, site,
        "Repeated original template in a cached specialization-creation path");
    plan->creation_witness = current;
    plan->closing_edge = closing;
    plan->rejected_origin = origin;
    plan->rejected_site = site;
    plan->rejected_path = closing->path->parent;
    plan->witness_arena = *scratch;
    source_arena_init(scratch, plan->arena.allocator);
    const ProjectModule *definition = origin->definition;
    plan->related[0] = (DiagnosticRelated){.location = {.span = {definition->source->text, 0, 0}},
        .role = "repeated source template", .name = definition->source->logical_path};
    plan->related[1] = (DiagnosticRelated){.location = {.span = closing->path->site},
        .role = "closing cached creation edge", .name = definition->source->logical_path};
    plan->error.related = plan->related;
    plan->error.related_count = 2;
    return false;
}

/** Reachability needs one witness per full vertex, not every simple path.
 * Check the target label before visited suppression so self edges also fail.
 * Ordinary-only cycles terminate through the same visited array and are allowed.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) checks the exact path predicate.
 */
static bool search(StaticGuardPlan *plan, StaticCreationVertex *start,
    StaticCreationSearch *entries, StaticCreationSearch **seen, SourceArena *scratch) {
    memset(seen, 0, plan->vertex_count * sizeof(*seen));
    entries[0] = (StaticCreationSearch){.vertex = start};
    seen[start->ordinal] = &entries[0];
    size_t count = 1;
    for (size_t front = 0; front < count; front++) {
        StaticCreationSearch *current = &entries[front];
        for (StaticCreationEdge *edge = current->vertex->first; edge; edge = edge->next) {
            if (edge->target->value->as.nominal.origin == start->value->as.nominal.origin) {
                return reject(plan, current, edge, scratch);
            }
            size_t index = edge->target->ordinal;
            if (seen[index] != NULL) { continue; }
            entries[count] = (StaticCreationSearch){.vertex = edge->target, .parent = current, .edge = edge};
            seen[index] = &entries[count++];
        }
    }
    return true;
}

/** A forbidden path exists iff one template vertex can reach a vertex with
 * the same original source label by at least one edge. Search each such start
 * over concrete keys; never collapse unrelated specializations into source nodes.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves cache equivalence.
 */
bool static_guard_creation_graph(StaticGuardPlan *plan) {
    bool templates = false;
    for (const StaticCreationVertex *vertex = plan->first_vertex; vertex; vertex = vertex->next) {
        if (vertex->value->as.nominal.origin->parameter_count != 0) { templates = true; break; }
    }
    if (!templates) { return true; }
    if (plan->vertex_count > SIZE_MAX / sizeof(StaticCreationSearch) ||
        plan->vertex_count > SIZE_MAX / sizeof(StaticCreationSearch *)) {
        return static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, (SourceSpan){0},
            "Concrete creation search exceeds representable storage");
    }
    SourceArena scratch;
    source_arena_init(&scratch, plan->arena.allocator);
    StaticCreationSearch *entries = source_arena_allocate(&scratch, plan->vertex_count * sizeof(*entries));
    StaticCreationSearch **seen = entries ? source_arena_allocate(&scratch, plan->vertex_count * sizeof(*seen)) : NULL;
    bool success = entries != NULL && seen != NULL;
    if (!success) {
        (void)static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, (SourceSpan){0},
            "Concrete creation search could not allocate scratch");
    }
    for (StaticCreationVertex *vertex = plan->first_vertex; success && vertex; vertex = vertex->next) {
        if (vertex->value->as.nominal.origin->parameter_count == 0) { continue; }
        success = search(plan, vertex, entries, seen, &scratch);
    }
    source_arena_destroy(&scratch);
    return success;
}
