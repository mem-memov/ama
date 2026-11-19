/** @file
 * @brief Join original field Type terms only from proven owner-expression congruence.
 * [TPL-04](../../../../../../spec/language/templates.md#tpl-04) requires one field Type per owner across uses.
 * [TPL-10](../../../../../../spec/language/templates.md#tpl-10) permits exact symbolic equations.
 * [TPL-11](../../../../../../spec/language/templates.md#tpl-11) still reserves expansion for closed repeated keys.
 */
#include "../../internal.h"

/** Corresponding fields have the same original immutable RuntimeModule order.
 * A proven equal owner denotes the same field Type under every completion; this
 * is congruence, not injectivity or a concrete specialization-cache publication.
 * [TPL-04](../../../../../../spec/language/templates.md#tpl-04) forbids per-use disagreement.
 */
static bool fields(StaticConnectionPlan *plan, const StaticTypedScope *left, const StaticTypedScope *right) {
    const StaticTypedField *other = right->first_field;
    StaticConstraintSolver *solver = plan->constants->publications->results->guards->applications->solver;
    for (const StaticTypedField *field = left->first_field; field; field = field->next, other = other->next) {
        if (other == NULL || field->definition != other->definition) {
            return static_connection_fail(plan, STATIC_CONNECTION_INVALID, field->definition->site, "Equal owner scopes have inconsistent original field identities");
        }
        if (!static_constraint_equate(solver, field->type, other->type, field->definition->site)) {
            (void)static_connection_dependencies(plan); return false;
        }
    }
    if (other) { return static_connection_fail(plan, STATIC_CONNECTION_INVALID, other->definition->site, "Equal owner scopes have inconsistent field cardinality"); }
    return true;
}

/** Scan each finite pair once for this attempt. Later genuine equations may
 * establish more congruences, so the caller retries only on mathematical progress.
 * All incoming actual contexts remain intact and are not admitted by this join.
 * [TPL-10](../../../../../../spec/language/templates.md#tpl-10) separates equality facts from required closure.
 */
bool static_connection_merge_fields(StaticConnectionPlan *plan, bool *output) {
    if (!static_connection_dependencies(plan)) { return false; }
    if (output == NULL) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, (SourceSpan){0}, "Missing field-congruence progress receiver");
    }
    StaticConstraintSolver *solver = plan->constants->publications->results->guards->applications->solver;
    size_t before = solver->equation_count;
    for (const StaticTypedScope *left = plan->first_scope; left; left = left->next) {
        for (const StaticTypedScope *right = left->next; right; right = right->next) {
            if (left->runtime != right->runtime) { continue; }
            bool equivalent;
            if (!static_constraint_equivalent(solver, left->owner, right->owner, &equivalent)) {
                (void)static_connection_dependencies(plan); return false;
            }
            if (equivalent && !fields(plan, left, right)) { return false; }
        }
    }
    *output = solver->equation_count != before;
    return true;
}
