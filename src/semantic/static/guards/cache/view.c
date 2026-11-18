/** @file
 * @brief Retain unresolved required scopes in cache-path validation before their complete keys exist.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves paths between closed unequal endpoints.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves symbolic occurrences.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) bounds scratch ownership.
 */
#include <stdint.h>
#include "view.h"
#include "../internal.h"

/** Pending scope capacity is counted from actual required frames, not all
 * metadata owners. All-concrete graphs retain O(V) node storage.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) excludes mere metadata capture edges.
 */
static bool capacity(StaticGuardPlan *plan, size_t *output) {
    size_t pending = 0;
    for (const StaticCreationPath *path = plan->first_path; path; path = path->next) {
        if (path->vertex != NULL) { continue; }
        if (pending == SIZE_MAX) {
            (void)static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, path->site, "Symbolic creation scope count exceeds storage");
            return false;
        }
        pending++;
    }
    if (pending > SIZE_MAX - plan->vertex_count ||
        pending + plan->vertex_count > SIZE_MAX / sizeof(StaticCreationViewNode)) {
        (void)static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, (SourceSpan){0},
            "Creation validation view exceeds representable storage");
        return false;
    }
    *output = pending + plan->vertex_count;
    return true;
}

/** Exact scope identity is sufficient before closure; only normalized complete
 * keys permit joining different scoped owners. All original frames remain stored.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves unresolved environments.
 */
bool static_creation_view(StaticGuardPlan *plan, SourceArena *scratch, StaticCreationView *output) {
    size_t maximum;
    if (!capacity(plan, &maximum)) { return false; }
    StaticCreationView view = {.count = plan->vertex_count};
    if (maximum == 0) { *output = view; return true; }
    view.nodes = source_arena_allocate(scratch, maximum * sizeof(*view.nodes));
    if (view.nodes == NULL) {
        return static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, (SourceSpan){0}, "Creation validation view could not allocate storage");
    }
    size_t actual = 0;
    for (StaticCreationVertex *vertex = plan->first_vertex; vertex; vertex = vertex->next) {
        if (vertex->ordinal >= plan->vertex_count) {
            return static_guard_fail(plan, STATIC_GUARD_INVALID, (SourceSpan){0}, "Invalid concrete creation vertex ordinal");
        }
        view.nodes[vertex->ordinal] = (StaticCreationViewNode){.vertex = vertex, .origin = vertex->value->as.nominal.origin};
        actual++;
    }
    if (actual != plan->vertex_count) {
        return static_guard_fail(plan, STATIC_GUARD_INVALID, (SourceSpan){0}, "Inconsistent concrete creation vertex count");
    }
    for (const StaticCreationPath *path = plan->first_path; path; path = path->next) {
        if (path->parent && (path->vertex == NULL || path->parent->vertex == NULL)) { view.partial_edges = true; }
        if (path->vertex != NULL) { continue; }
        size_t index = plan->vertex_count;
        while (index < view.count && view.nodes[index].owner != path->owner) { index++; }
        if (index != view.count) { continue; }
        view.nodes[view.count++] = (StaticCreationViewNode){.owner = path->owner, .origin = path->owner->environment->definition};
    }
    *output = view;
    return true;
}

/** Parent/target class lookup uses exact published identity, never source-name
 * or signature resemblance. Private frame invariants ensure a class exists.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves real edges.
 */
bool static_creation_view_path(StaticGuardPlan *plan, const StaticCreationView *view,
    const StaticCreationPath *path, size_t *output) {
    if (path->vertex != NULL && path->vertex->ordinal < view->count &&
        view->nodes[path->vertex->ordinal].vertex == path->vertex) {
        *output = path->vertex->ordinal;
        return true;
    }
    if (path->vertex == NULL) {
        for (size_t index = plan->vertex_count; index < view->count; index++) {
            if (view->nodes[index].owner == path->owner) { *output = index; return true; }
        }
    }
    return static_guard_fail(plan, STATIC_GUARD_INVALID, path->site, "Creation frame has no exact validation class");
}
