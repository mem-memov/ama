/** @file
 * @brief Recover an unknown constructor from one known nominal result and explicitly named inputs.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) retains all fixed captures.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) permits unique recovery without search.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial maps.
 */
#include "recovery/internal.h"

/** Record the result equality before the structural pass, so any newly known
 * head is checked against existing facts instead of overwritten. Finite unknown
 * applications remain explicit constraints for their later orchestration owner.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) preserves all equations.
 */
bool static_application_recover(StaticApplicationPlan *plan, const StaticTermApplication *source,
    const StaticTerm *actual, const StaticApplicationRecord **output) {
    if (!static_application_dependencies(plan)) { return false; }
    const StaticTermApplication *registered = plan->terms->first_application;
    while (registered != NULL && registered != source) { registered = registered->next; }
    if (registered == NULL || output == NULL || !static_term_has_value(plan->terms, actual)) {
        return static_application_fail(plan, STATIC_APPLY_INVALID, (SourceSpan){0},
            "Recovery requires an owned source application and actual value", NULL);
    }
    StaticApplicationContext context;
    if (!static_application_context(plan, source->environment, source->source->expression, &context)) { return false; }
    SourceSpan site = source->source->expression->span;
    const StaticTerm *result;
    if (!static_term_expression_result(plan->terms, source->environment, source->source->expression, &result) ||
        !static_constraint_equate(plan->solver, result, actual, site)) {
        (void)static_application_dependencies(plan); return false;
    }
    StaticConstraintOutcome solved;
    if (!static_constraint_solve(plan->solver, &solved)) { (void)static_application_dependencies(plan); return false; }
    const StaticTerm *head = static_constraint_root(plan->solver, source->head);
    actual = static_constraint_root(plan->solver, actual);
    if (head->kind == STATIC_TERM_VARIABLE && actual->kind == STATIC_TERM_NOMINAL) {
        if (source->input_count != 0) {
            if (!static_recovery_constructor(plan, source, head, actual, site)) { return false; }
        } else if (!static_constraint_equate(plan->solver, head, actual, site)) {
            (void)static_application_dependencies(plan);
            return false;
        }
    }
    return static_application_source(plan, source, output);
}
