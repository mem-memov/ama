/** @file
 * @brief Normalize original static operator equations without entering or expanding producer definitions.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves actual origin/captures.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) permits only independent nominal recovery evidence.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) leaves actual creation checks to guarded uses.
 */
#include "../internal.h"
#include "../../constraints/internal.h"

/** Unknown-origin snapshots cannot be rechecked into a different interface.
 * Replace one only when the head has independently acquired a nominal origin;
 * otherwise retain its original request provenance without manufacturing work.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) forbids origin guessing.
 */
bool static_result_application(StaticResultPlan *plan, const StaticResultSource *source,
    const StaticTermApplication *prepared, const StaticApplicationRecord **application) {
    StaticApplicationPlan *applications = plan->guards->applications;
    StaticConstraintOutcome solved;
    if (!static_constraint_solve(applications->solver, &solved)) {
        (void)static_result_dependencies(plan); return false;
    }
    const StaticTerm *head = static_constraint_root(applications->solver, source->head);
    const StaticTerm *result = static_constraint_root(applications->solver, source->value);
    StaticCategory category;
    const StaticSignature *signature;
    static_constraint_description(applications->solver, source->head, &category, &signature);
    (void)signature;
    if (head->kind == STATIC_TERM_VARIABLE && result->kind == STATIC_TERM_NOMINAL &&
        (category == STATIC_CATEGORY_TEMPLATE ||
            (category == STATIC_CATEGORY_TYPE && prepared->input_count == 0))) {
        const StaticApplicationRecord *record;
        if (!static_application_recover(applications, prepared, result, &record)) {
            (void)static_result_dependencies(plan); return false;
        }
        *application = record;
        return true;
    }
    bool newly_known = (*application) != NULL && (*application)->pending_origin &&
        static_constraint_root(applications->solver, source->head)->kind == STATIC_TERM_NOMINAL;
    if ((*application) == NULL || newly_known) {
        const StaticApplicationRecord *record;
        if (!static_application_source(applications, prepared, &record)) {
            (void)static_result_dependencies(plan); return false;
        }
        *application = record;
    }
    return true;
}
