/** @file
 * @brief Preserve every original incoming creation use as its endpoint keys become concrete.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) retains provenance across cache merging.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 */
#include <stdint.h>
#include "../internal.h"

/** Each immutable frame contributes at most one concrete edge. Unknown parent
 * keys defer the edge without discarding symbolic active-ancestry evidence.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves incoming uses.
 */
bool static_guard_edges(StaticGuardPlan *plan) {
    for (StaticCreationPath *path = plan->first_path; path; path = path->next) {
        if (path->edge != NULL || path->parent == NULL || path->vertex == NULL || path->parent->vertex == NULL) { continue; }
        if (plan->edge_count == SIZE_MAX) {
            return static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, path->site,
                "Concrete creation edge count exceeds representable storage");
        }
        StaticCreationEdge *edge = static_guard_allocate(plan, sizeof(*edge));
        if (edge == NULL) { return false; }
        *edge = (StaticCreationEdge){.target = path->vertex, .path = path};
        StaticCreationVertex *source = path->parent->vertex;
        if (source->last != NULL) { source->last->next = edge; }
        else { source->first = edge; }
        source->last = edge;
        path->edge = edge;
        plan->edge_count++;
    }
    return true;
}
