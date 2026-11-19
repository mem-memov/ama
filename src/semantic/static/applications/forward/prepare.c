/** @file
 * @brief Prepare actual-origin forward applications before publishing immutable snapshots.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves Type/Template distinctions.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) requires producer closure.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial preparation.
 */
#include <stdint.h>
#include "../internal.h"

/** Bare references preserve existing values in partial contexts; a known Type
 * also needs no constructor origin to serve as a symbolic runtime type. Unknown
 * Template origins are never invented for omission or input processing.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) distinguishes application from forwarding.
 */
static bool unknown(StaticApplicationPlan *plan, StaticApplicationRecord *record) {
    StaticCategory category;
    const StaticSignature *signature;
    static_application_description(plan, record->head, &category, &signature);
    (void)signature;
    if (record->input_count != 0 && category == STATIC_CATEGORY_TYPE) {
        return static_application_fail(plan, STATIC_APPLY_CONFLICT, record->context.site,
            "A Type cannot receive static input bindings", NULL);
    }
    if (record->input_count == 0 && (record->context.policy == STATIC_OMIT_PARTIAL || category == STATIC_CATEGORY_TYPE)) {
        record->value = record->head;
        record->pending_origin = false;
        return true;
    }
    if (record->input_count == 0 && category == STATIC_CATEGORY_TEMPLATE &&
        record->context.policy == STATIC_OMIT_NAMESPACE) {
        return static_application_fail(plan, STATIC_APPLY_CONFLICT, record->context.site,
            "This type expression requires explicit coverage of every residual formal", NULL);
    }
    record->pending_origin = true;
    return true;
}

/** Snapshot source inputs, normalize by actual head metadata, and publish only
 * after preparation/contracts succeed. Pending candidates remain compiler facts,
 * never executable specializations or projected outputs.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) defines structural substitution.
 */
bool static_application_forward(StaticApplicationPlan *plan, const StaticTerm *head,
    const StaticApplicationInput *inputs, size_t count, StaticApplicationContext context,
    const StaticApplicationRecord **output) {
    if (!static_application_dependencies(plan)) { return false; }
    if (output == NULL || !static_term_has_value(plan->terms, head)) {
        return static_application_fail(plan, STATIC_APPLY_INVALID, context.site,
            "Forward application requires an owned head and output receiver", NULL);
    }
    if (!static_application_context_valid(plan, context)) { return false; }
    StaticConstraintOutcome solved;
    if (!static_constraint_solve(plan->solver, &solved)) { (void)static_application_dependencies(plan); return false; }
    const StaticTerm *constructor = static_constraint_root(plan->solver, head);
    if (!static_application_inputs_valid(plan, constructor, inputs, count, context.site)) { return false; }
    if (plan->count == SIZE_MAX) {
        return static_application_fail(plan, STATIC_APPLY_OUT_OF_MEMORY, context.site,
            "Static application count exceeds representable storage", NULL);
    }
    StaticApplicationRecord *record = static_application_allocate(plan, sizeof(*record));
    if (record == NULL) { return false; }
    *record = (StaticApplicationRecord){.head = head, .constructor = constructor, .context = context};
    for (size_t index = 0; index < count; index++) {
        StaticApplicationInput input = inputs[index];
        input.generated = false;
        if (!static_application_input(plan, record, input)) { return false; }
    }
    if (constructor->kind == STATIC_TERM_VARIABLE) {
        if (!unknown(plan, record)) { return false; }
    } else if (!static_application_merge(plan, record)) { return false; }
    if (context.result != NULL && record->value != NULL &&
        !static_constraint_equate(plan->solver, context.result, record->value, context.site)) {
        (void)static_application_dependencies(plan); return false;
    }
    if (!static_constraint_solve(plan->solver, &solved)) {
        (void)static_application_dependencies(plan); return false;
    }
    if (record->value != NULL && context.policy != STATIC_OMIT_PARTIAL &&
        !static_constraint_require(plan->solver, record->value, context.site)) {
        (void)static_application_dependencies(plan);
        return false;
    }
    StaticApplicationOutcome outcome;
    if (!static_application_validate(plan, record, &outcome)) { return false; }
    if (plan->last != NULL) { plan->last->next = record; }
    else { plan->first = record; }
    plan->last = record;
    plan->count++;
    *output = record;
    return true;
}
