/** @file
 * @brief Validate actual annotation paths and complete field Types without admitting an ordinary body.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) requires declared/inferred/local field Types.
 * [TPL-11](../../../../../../spec/language/templates.md#tpl-11) retains real incoming annotation ancestry.
 */
#include "../../internal.h"

/** Actual source uses are distinct from source-only facts even when their Type
 * variables coincide. Pending paths remain required; no metadata-only root is
 * promoted into an actual caller context.
 * [TPL-11](../../../../../../spec/language/templates.md#tpl-11) prevents a metadata/cache bypass.
 */
static bool annotation(StaticConnectionPlan *plan, const StaticResultContext *actual,
    const StaticTypedField *field, bool *ready) {
    if (!field->type_source) { return true; }
    StaticOrdinaryType *use;
    StaticResultOutcome outcome;
    if (!static_ordinary_type(plan->constants->publications, actual, field->definition->type_expression, &use) ||
        !static_ordinary_evaluate(plan->constants->publications, use, &outcome)) {
        (void)static_connection_dependencies(plan); return false;
    }
    *ready &= outcome == STATIC_RESULT_READY;
    return true;
}

/** Continue across Pending fields so unrelated known annotations remain usable;
 * fail only on a genuine conflict/refusal. Own-body equation generation is a
 * separate later operation and may refine these outstanding requirements.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) separates inference from final closure.
 */
bool static_connection_validate_fields(StaticConnectionPlan *plan, const StaticResultContext *actual,
    StaticResultOutcome *output) {
    if (!static_connection_dependencies(plan)) { return false; }
    if (!output) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, (SourceSpan){0}, "Missing actual field-validation outcome receiver");
    }
    const StaticTypedScope *scope;
    if (!static_connection_context(plan, actual, &scope)) { return false; }
    StaticConstraintSolver *solver = plan->constants->publications->results->guards->applications->solver;
    bool ready = true;
    for (const StaticTypedField *field = scope->first_field; field; field = field->next) {
        if (!annotation(plan, actual, field, &ready)) { return false; }
        const StaticTerm *value;
        StaticConstraintOutcome outcome;
        if (!static_constraint_value(solver, field->type, &value, &outcome)) {
            (void)static_connection_dependencies(plan); return false;
        }
        ready &= outcome == STATIC_CONSTRAINT_COMPLETE;
    }
    *output = ready ? STATIC_RESULT_READY : STATIC_RESULT_PENDING;
    return true;
}
