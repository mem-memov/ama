/** @file
 * @brief Normalize original static operator equations without entering or expanding producer definitions.
 * [TPL-04](../../../../../spec/language/templates.md#tpl-04) instantiates omitted ordinary field parameters.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves actual origin/captures.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) permits only independent nominal recovery evidence.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) leaves actual creation checks to guarded uses.
 */
#include "../internal.h"
#include "../../constraints/internal.h"

/** Unknown-origin snapshots cannot be rechecked into a different interface.
 * Replace one only when the head has independently acquired a nominal origin;
 * otherwise retain its original request provenance without manufacturing work.
 * [TPL-04](../../../../../spec/language/templates.md#tpl-04) instantiates an ordinary generic field path rather than equating it with its residual constructor.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) forbids origin guessing.
 */
bool static_result_application(StaticResultPlan *plan, const StaticResultSource *source,
    const StaticTermApplication *prepared, const StaticApplicationRecord **application) {
    StaticApplicationPlan *applications = plan->guards->applications;
    StaticApplicationContext context;
    if (!static_application_context(applications, prepared->environment, source->expression, &context)) {
        (void)static_result_dependencies(plan); return false;
    }
    /* TPL-04 instantiates a generic module path in an ordinary field. Its
     * omitted formals become field inference variables, so its value cannot be
     * equated with the residual constructor. Bare Type placeholders still
     * forward their supplied value; static graph partial values keep identity. */
    bool field_constructor = context.policy == STATIC_OMIT_FIELD &&
        source->expression->kind == AST_TYPE_MODULE_PATH &&
        source->head->kind == STATIC_TERM_NOMINAL && source->head->category == STATIC_CATEGORY_TEMPLATE;
    if (!field_constructor && prepared->input_count == 0 && prepared->source->output_count == 0 &&
        !static_constraint_equate(applications->solver, source->head, source->value, source->expression->span)) {
        (void)static_result_dependencies(plan); return false;
    }
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
