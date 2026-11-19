/** @file
 * @brief Constrain traversal cells by their original collection element Types.
 * [FLOW-05](../../../../../../spec/language/lists.md#flow-05) requires element Type equality even for empty lists.
 * [NAME-06](../../../../../../spec/language/declarations.md#name-06) fixes lexical binding identity before solving.
 */
#include "../../internal.h"

/** Retained traversal identities prevent later module names or sibling locals
 * from changing an earlier binding. Equality is symmetric and carries no runtime
 * element existence claim; later anchors refine either endpoint. Only actual
 * incoming contexts enable own-body equations, never source-only metadata.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) keeps binding decisions independent of inference.
 */
bool static_connection_foreach(StaticConnectionPlan *plan, const StaticResultContext *actual, bool *output) {
    if (!static_connection_dependencies(plan)) { return false; }
    if (!output) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, (SourceSpan){0}, "Missing foreach equation progress receiver");
    }
    const StaticTypedScope *scope;
    if (!static_connection_context(plan, actual, &scope)) { return false; }
    StaticConstraintSolver *solver = plan->constants->publications->results->guards->applications->solver;
    size_t before = solver->equation_count;
    for (const StaticRuntimeForeach *loop = scope->runtime->first_foreach; loop; loop = loop->next) {
        const StaticTypedField *collection, *binding;
        if (!static_connection_field(plan, scope, loop->collection, &collection) ||
            !static_connection_field(plan, scope, loop->binding, &binding)) { return false; }
        if (!static_constraint_equate(solver, collection->type, binding->type, loop->syntax->as.traversal.binder)) {
            (void)static_connection_dependencies(plan); return false;
        }
    }
    *output = solver->equation_count != before;
    return true;
}
