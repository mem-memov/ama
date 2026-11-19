/** @file
 * @brief Intern full closed owner creation keys without merging source-equal specializations.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves full concrete identity.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 */
#include <assert.h>
#include <stdint.h>
#include "../internal.h"

/** Only successful complete owner normalization authorizes this internal call;
 * metadata groundness alone is not a normalization or admission proof.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) uses normalized keys.
 */
bool static_guard_vertex(StaticGuardPlan *plan, const StaticTermEnvironment *key, StaticCreationVertex **output) {
    assert(key->owner_type != NULL || key->producer_applied);
    const StaticTerm *value = key->owner_type;
    assert(value == NULL || (value->kind == STATIC_TERM_NOMINAL &&
        value->category == STATIC_CATEGORY_TYPE && value->ground));
    for (const StaticTermCapture *input = key->producer_inputs; input; input = input->next) {
        assert(input->value->ground);
    }
    for (StaticCreationVertex *vertex = plan->first_vertex; vertex; vertex = vertex->next) {
        if (vertex->key == key) { *output = vertex; return true; }
    }
    if (plan->vertex_count == SIZE_MAX) {
        return static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, (SourceSpan){0},
            "Concrete creation vertex count exceeds representable storage");
    }
    StaticCreationVertex *vertex = static_guard_allocate(plan, sizeof(*vertex));
    if (vertex == NULL) { return false; }
    *vertex = (StaticCreationVertex){.value = value, .key = key, .ordinal = plan->vertex_count, .next = plan->first_vertex};
    plan->first_vertex = vertex;
    plan->vertex_count++;
    *output = vertex;
    return true;
}
