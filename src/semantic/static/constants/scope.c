/** @file
 * @brief Validate explicit required namespace contexts without using them to infer namespaces.
 * [CONST-03](../../../../spec/language/declarations.md#const-03) requires independent lookup closure.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves fixed-Type reuse without creation edges.
 */
#include "internal.h"

/** The caller-supplied checked context is verified after namespace inference,
 * never used as evidence to select its nominal Type. Plain supplied-Type reuse
 * needs actual registry provenance, not a same-named implicit root.
 * [CONST-03](../../../../spec/language/declarations.md#const-03) fixes lookup ownership.
 */
bool static_constant_scope(StaticConstantPlan *plan, const StaticResultContext *context, const StaticTerm *type) {
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    if (!static_result_has_context(plan->publications->results, context)) {
        return static_constant_fail(plan, STATIC_CONSTANT_INVALID, type->site, "Constant namespace has no checked actual scope");
    }
    if (!static_constraint_value(plan->publications->results->guards->applications->solver,
        context->creation->owner->value, &value, &outcome)) {
        (void)static_constant_dependencies(plan); return false;
    }
    if (outcome != STATIC_CONSTRAINT_COMPLETE || value != type) {
        return static_constant_fail(plan, STATIC_CONSTANT_INVALID, type->site, "Checked constant namespace scope does not match its independently determined Type");
    }
    return true;
}
