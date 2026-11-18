/** @file
 * @brief Intern full closed nominal creation keys without merging source-equal specializations.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves full concrete identity.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 */
#include <assert.h>
#include <stdint.h>
#include "../internal.h"

/** Only a successful T176 complete value query authorizes this internal call;
 * metadata groundness alone is not a normalization or admission proof.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) uses normalized keys.
 */
bool static_guard_vertex(StaticGuardPlan *plan, const StaticTerm *value, StaticCreationVertex **output) {
    assert(value->kind == STATIC_TERM_NOMINAL && value->category == STATIC_CATEGORY_TYPE && value->ground);
    for (StaticCreationVertex *vertex = plan->first_vertex; vertex; vertex = vertex->next) {
        if (vertex->value == value) { *output = vertex; return true; }
    }
    if (plan->vertex_count == SIZE_MAX) {
        return static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, (SourceSpan){0},
            "Concrete creation vertex count exceeds representable storage");
    }
    StaticCreationVertex *vertex = static_guard_allocate(plan, sizeof(*vertex));
    if (vertex == NULL) { return false; }
    *vertex = (StaticCreationVertex){.value = value, .ordinal = plan->vertex_count, .next = plan->first_vertex};
    plan->first_vertex = vertex;
    plan->vertex_count++;
    *output = vertex;
    return true;
}
