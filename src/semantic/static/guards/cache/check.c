/** @file
 * @brief Reject repeated known source templates across concrete caches and unresolved scoped paths.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) checks before key closure.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains symbolic ancestry.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) releases successful search scratch.
 * [COMP-04](../../../../../spec/implementation/compiler.md#comp-04) retains complete rejection witnesses.
 */
#include <stdint.h>
#include <string.h>
#include "view.h"
#include "../internal.h"

/** Transfer already allocated evidence without allocating to explain the error.
 * A closing path can still be symbolic and need not have a concrete edge yet.
 * [COMP-04](../../../../../spec/implementation/compiler.md#comp-04) retains actual source uses.
 */
static bool reject(StaticGuardPlan *plan, StaticCreationSearch *current,
    const StaticCreationViewNode *target, StaticCreationEdge *edge,
    const StaticCreationPath *path, SourceArena *scratch) {
    const StaticModule *origin = target->origin;
    (void)static_guard_fail(plan, STATIC_GUARD_CONFLICT, path->site,
        "Repeated original template in a cached specialization-creation path");
    plan->creation_witness = current;
    plan->closing_edge = edge;
    plan->closing_creation_path = path;
    plan->rejected_origin = origin;
    plan->rejected_site = path->site;
    plan->rejected_path = path->parent;
    plan->witness_arena = *scratch;
    source_arena_init(scratch, plan->arena.allocator);
    const ProjectModule *definition = origin->definition;
    plan->related[0] = (DiagnosticRelated){.location = {.span = {definition->source->text, 0, 0}},
        .role = "repeated source template", .name = definition->source->logical_path};
    plan->related[1] = (DiagnosticRelated){.location = {.span = path->site},
        .role = "closing cached creation edge", .name = definition->source->logical_path};
    plan->error.related = plan->related;
    plan->error.related_count = 2;
    return false;
}

/** Check a real target's known source before visited suppression. Each exact
 * validation class enters the queue once; provenance remains on original paths.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) checks nonempty paths.
 */
static bool visit(StaticGuardPlan *plan, const StaticCreationView *view, const StaticModule *origin,
    size_t target, StaticCreationSearch *current, StaticCreationEdge *edge, const StaticCreationPath *path,
    StaticCreationSearch *entries, StaticCreationSearch **seen, size_t *count, SourceArena *scratch) {
    const StaticCreationViewNode *node = &view->nodes[target];
    if (node->origin == origin) { return reject(plan, current, node, edge, path, scratch); }
    if (seen[target] != NULL) { return true; }
    entries[*count] = (StaticCreationSearch){.vertex = node->vertex, .owner = node->owner, .origin = node->origin,
        .ordinal = target, .parent = current, .edge = edge, .path = path};
    seen[target] = &entries[(*count)++];
    return true;
}

/** Concrete adjacency is reused directly. Edges with an unresolved endpoint
 * are retained occurrence paths, grouped only by exact scoped owner identity.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) checks known origins before closure.
 */
static bool search(StaticGuardPlan *plan, const StaticCreationView *view, size_t start,
    StaticCreationSearch *entries, StaticCreationSearch **seen, SourceArena *scratch) {
    memset(seen, 0, view->count * sizeof(*seen));
    const StaticCreationViewNode *root = &view->nodes[start];
    entries[0] = (StaticCreationSearch){.vertex = root->vertex, .owner = root->owner,
        .origin = root->origin, .ordinal = start};
    seen[start] = &entries[0];
    size_t count = 1;
    for (size_t front = 0; front < count; front++) {
        StaticCreationSearch *current = &entries[front];
        const StaticCreationViewNode *node = &view->nodes[current->ordinal];
        if (node->vertex != NULL) {
            for (StaticCreationEdge *edge = node->vertex->first; edge; edge = edge->next) {
                if (!visit(plan, view, root->origin, edge->target->ordinal, current, edge, edge->path,
                    entries, seen, &count, scratch)) { return false; }
            }
        }
        if (!view->partial_edges) { continue; }
        for (const StaticCreationPath *path = plan->first_path; path; path = path->next) {
            if (path->parent == NULL || (path->vertex != NULL && path->parent->vertex != NULL)) { continue; }
            bool matches = path->parent->vertex != NULL ? node->vertex == path->parent->vertex : node->owner == path->parent->owner;
            if (!matches) { continue; }
            size_t target;
            if (!static_creation_view_path(plan, view, path, &target) ||
                !visit(plan, view, root->origin, target, current, NULL, path,
                    entries, seen, &count, scratch)) { return false; }
        }
    }
    return true;
}

/** Same-origin reachability checks both already complete graph paths and
 * known symbolic prefixes/suffixes. Exact scope classes are validation views,
 * not prematurely published concrete cache keys. No path/depth budget is used.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) validates all incoming paths.
 */
bool static_guard_creation_graph(StaticGuardPlan *plan) {
    SourceArena scratch;
    source_arena_init(&scratch, plan->arena.allocator);
    StaticCreationView view;
    bool success = static_creation_view(plan, &scratch, &view);
    bool templates = false;
    if (success) {
        for (size_t index = 0; index < view.count; index++) {
            if (view.nodes[index].origin->parameter_count != 0) { templates = true; break; }
        }
    }
    if (success && templates) {
        if (view.count > SIZE_MAX / sizeof(StaticCreationSearch) || view.count > SIZE_MAX / sizeof(StaticCreationSearch *)) {
            success = static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, (SourceSpan){0},
                "Creation path search exceeds representable storage");
        } else {
            StaticCreationSearch *entries = source_arena_allocate(&scratch, view.count * sizeof(*entries));
            StaticCreationSearch **seen = entries ? source_arena_allocate(&scratch, view.count * sizeof(*seen)) : NULL;
            if (entries == NULL || seen == NULL) {
                success = static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, (SourceSpan){0},
                    "Creation path search could not allocate scratch");
            }
            for (size_t index = 0; success && index < view.count; index++) {
                if (view.nodes[index].origin->parameter_count == 0) { continue; }
                success = search(plan, &view, index, entries, seen, &scratch);
            }
        }
    }
    source_arena_destroy(&scratch);
    return success;
}
